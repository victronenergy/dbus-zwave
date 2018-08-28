#include <stdio.h>
#include <stdlib.h>
#include <string>

extern "C" {
#include <velib/base/base.h>
#include <velib/platform/console.h>
#include <velib/platform/plt.h>
#include <velib/types/ve_dbus_item.h>
#include <velib/types/ve_item.h>
#include <velib/types/ve_values.h>
#include <velib/utils/ve_logger.h>
}

#include <Defs.h>
#include <Driver.h>
#include <Manager.h>
#include <Notification.h>
#include <Options.h>
#include <platform/Log.h>
#include <value_classes/ValueID.h>

#include "configurators/dz_aeotec_zw095.hpp"
#include "dz_button.hpp"
#include "dz_commandclass.hpp"
#include "dz_constvalue.hpp"
#include "dz_driver.hpp"
#include "dz_item.hpp"
#include "dz_logger.hpp"
#include "dz_node.hpp"
#include "dz_setting.hpp"
#include "dz_value.hpp"
#include "values/dz_gridmeter.hpp"
#include "values/dz_temperature.hpp"

using OpenZWave::Manager;
using OpenZWave::Notification;
using OpenZWave::Options;
using OpenZWave::ValueID;
using std::string;

static bool             publishZwaveData = false;
static pthread_cond_t   initCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t  initMutex = PTHREAD_MUTEX_INITIALIZER;
static volatile bool    initComplete = false;
static volatile bool    initFailed = false;
struct VeDbus*          dbusConnection;

void onZwaveNotification(const Notification* _notification, void* _context)
{
    VE_UNUSED(_context);
    switch (_notification->GetType())
    {
        case Notification::Type_AwakeNodesQueried:
        case Notification::Type_AllNodesQueried:
        case Notification::Type_AllNodesQueriedSomeDead:
        {
            initComplete = true;
            pthread_cond_broadcast(&initCond);
            break;
        }

        case Notification::Type_DriverFailed:
        {
            if (!initComplete)
            {
                initFailed = true;
                pthread_cond_broadcast(&initCond);
            }
            else
            {
                logE("task", "zwave connection failed");
                pltExit(1);
            }
            break;
        }

        case Notification::Type_DriverReady:
        {
            if (publishZwaveData)
            {
                (new DZDriver(_notification->GetHomeId()))->publish();
            }

            // Add node button
            uint32 zwaveHomeId = _notification->GetHomeId();
            (new DZButton(DZItem::defaultServiceName, "AddNode", [=]() {
                Manager::Get()->AddNode(zwaveHomeId, true);
            }))->publish();
            (new DZButton(DZItem::defaultServiceName, "RemoveNode", [=]() {
                Manager::Get()->RemoveNode(zwaveHomeId);
            }))->publish();
            break;
        }

        case Notification::Type_NodeAdded:
        {
            if (publishZwaveData)
            {
                ValueID zwaveValueId = _notification->GetValueID();
                if(zwaveValueId.GetNodeId() != Manager::Get()->GetControllerNodeId(zwaveValueId.GetHomeId()))
                {
                    (new DZNode(_notification->GetHomeId(), _notification->GetNodeId()))->publish();
                }
            }
            break;
        }

        case Notification::Type_ValueAdded:
        {
            ValueID zwaveValueId = _notification->GetValueID();
            if (publishZwaveData)
            {
                if(zwaveValueId.GetNodeId() != Manager::Get()->GetControllerNodeId(zwaveValueId.GetHomeId()))
                {
                    if (DZItem::get(_notification->GetHomeId(), _notification->GetNodeId(), zwaveValueId.GetCommandClassId()) == NULL)
                    {
                        (new DZCommandClass(_notification->GetHomeId(), _notification->GetNodeId(), zwaveValueId.GetCommandClassId()))->publish();
                    }

                    (new DZValue(zwaveValueId))->publish();
                }
            }

            // Grid meter
            if (DZGridMeter::handles(zwaveValueId))
            {
                (new DZGridMeter(zwaveValueId))->publish();
            }
            if (DZAeotecZw095::handles(zwaveValueId))
            {
                (new DZAeotecZw095(zwaveValueId))->bind(!initComplete);
            }

            // Temperature
            if (DZTemperature::handles(zwaveValueId))
            {
                (new DZTemperature(zwaveValueId))->publish();
            }
            break;
        }

        default:
        {
            break;
        }
    }
}

// velib links against these
extern "C" {

void taskInit(void)
{
    pthread_mutex_lock(&initMutex);

    // Set up Open-Zwave logger
    OpenZWave::Log::SetLoggingClass(new DZLogger());

    // Load system and user configuration
    Options::Create("config", "data", "");
    Options::Get()->AddOptionInt("PollInterval", 10 * 1000);
    Options::Get()->Lock();

    Manager::Create();
    Manager::Get()->AddWatcher(onZwaveNotification, NULL);
    Manager::Get()->AddDriver(pltGetSerialDevice());

    // Wait for successful init
    pthread_cond_wait(&initCond, &initMutex);
    if (initFailed)
    {
        logE("task", "zwave connection failed to initialize");
        pltExit(1);
    }

    // Publish information about the Z-Wave D-Bus service
    (new DZConstValue("com.victronenergy.zwave", "Mgmt/ProcessName", pltProgramName()))->publish();
    (new DZConstValue("com.victronenergy.zwave", "Mgmt/ProcessVersion", pltProgramVersion()))->publish();
    (new DZConstValue("com.victronenergy.zwave", "Mgmt/Connection", pltGetSerialDevice()))->publish();

    // Connect D-Bus to publish all the services
    DZItem::connectServices();
}

/*
* Note: only needed for the blocking version.
* not needed when using libevent.
*/
void taskUpdate(void)
{
    DZItem::updateDbusConnections();
}

/*
* Only needed if timeouts are used.
*/
void taskTick(void)
{
    VeItem* veRoot = veValueTree();
    veItemTick(veRoot);
}

char const *pltProgramVersion(void)
{
    return "dev";
}

struct option consoleOptions[]
{
    {"publish-zwave-data", no_argument, 0, 'z'}
};

void consoleUsage(char* program)
{
    printf("Usage: %s [OPTIONS...]\n", program);
    printf("\n");
    printf("OPTIONS:\n");
    printf("\n");
    printf("  -z, --publish-zwave-data\n");
    printf("   Publish \033[1mall\033[0m Z-Wave network data to the com.victronenergy.zwave D-Bus service.\n");
    printf("   Disabled by default.\n");
    printf("\n");
    pltOptionsUsage();
    printf("Victron Energy B.V.\n");
}

veBool consoleOption(int flag)
{
    if (flag == 'z')
    {
        publishZwaveData = true;
        return veTrue;
    }
    else
    {
        return veFalse;
    }
}

veBool consoleArgs(int argc, char *argv[])
{
    VE_UNUSED(argv);

    if (argc != 0)
    {
        printf("error - no arguments are expected - missing a '-' or '--'?\n");
        return veFalse;
    }

    return veTrue;
}

} // extern "C"
