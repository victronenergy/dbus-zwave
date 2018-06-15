#include <stdio.h>
#include <stdlib.h>
#include <string>

extern "C" {
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
#include <value_classes/ValueID.h>

#include "dz_commandclass.hpp"
#include "dz_constvalue.hpp"
#include "dz_driver.hpp"
#include "dz_item.hpp"
#include "dz_node.hpp"
#include "dz_setting.hpp"
#include "dz_value.hpp"
#include "values/dz_gridmeter.hpp"
#include "values/dz_temperature.hpp"
#include "configurators/dz_aeotec_zw095.hpp"

using OpenZWave::Manager;
using OpenZWave::Notification;
using OpenZWave::Options;
using OpenZWave::ValueID;
using std::string;

static pthread_cond_t   initCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t  initMutex = PTHREAD_MUTEX_INITIALIZER;
static volatile bool    initFailed = false;
struct VeDbus*          dbusConnection;

void onZwaveNotification(const Notification* _notification, void* _context)
{
    switch (_notification->GetType())
    {
        case Notification::Type_AwakeNodesQueried:
        case Notification::Type_AllNodesQueried:
        case Notification::Type_AllNodesQueriedSomeDead:
        {
            pthread_cond_broadcast(&initCond);
            break;
        }

        case Notification::Type_DriverFailed:
        {
            initFailed = true;
            pthread_cond_broadcast(&initCond);
            break;
        }

        case Notification::Type_DriverReady:
		{
            (new DZDriver(_notification->GetHomeId()))->publish();
            break;
        }

        case Notification::Type_NodeAdded:
        {
            ValueID zwaveValueId = _notification->GetValueID();
            if(zwaveValueId.GetNodeId() != Manager::Get()->GetControllerNodeId(zwaveValueId.GetHomeId()))
            {
                (new DZNode(_notification->GetHomeId(), _notification->GetNodeId()))->publish();
            }
            break;
        }

        case Notification::Type_ValueAdded:
        {
            ValueID zwaveValueId = _notification->GetValueID();
            if(zwaveValueId.GetNodeId() != Manager::Get()->GetControllerNodeId(zwaveValueId.GetHomeId()))
            {
                if (DZItem::get(_notification->GetHomeId(), _notification->GetNodeId(), zwaveValueId.GetCommandClassId()) == NULL)
                {
                    (new DZCommandClass(_notification->GetHomeId(), _notification->GetNodeId(), zwaveValueId.GetCommandClassId()))->publish();
                }

                (new DZValue(zwaveValueId))->publish();

                // Grid meter
                if (DZGridMeter::handles(zwaveValueId))
                {
                    (new DZGridMeter(zwaveValueId))->publish();
                }
                if (DZAeotecZw095::handles(zwaveValueId))
                {
                    (new DZAeotecZw095(zwaveValueId))->bind();
                }

                // Temperature
                if (DZTemperature::handles(zwaveValueId))
                {
                    (new DZTemperature(zwaveValueId))->publish();
                }
            }
            break;
        }

        default:
        {
            break;
        }
    }
}

extern "C" void taskInit(void)
{
    pthread_mutex_lock(&initMutex);

    // Velib logging
    #ifdef DEBUG
    veLogLevel(3);
    #endif

    // Configure OpenZWave
    Options::Create("config/", "", "");
    #ifdef DEBUG
    Options::Get()->AddOptionBool("ConsoleOutput", true);
    #else
    Options::Get()->AddOptionBool("ConsoleOutput", false);
    #endif
    Options::Get()->Lock();

    Manager::Create();
    Manager::Get()->AddWatcher(onZwaveNotification, NULL);

    // Add drivers
    Manager::Get()->AddDriver(pltGetSerialDevice());

    // Wait for successful init
    pthread_cond_wait(&initCond, &initMutex);
    if (initFailed)
    {
        logE("task", "zwave connection failed");
        pltExit(1);
    }

    // Publish information about the Z-Wave D-Bus service
    (new DZConstValue("com.victronenergy.zwave", "ProductName", "Victron Z-Wave Bridge"))->publish();
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
extern "C" void taskUpdate(void)
{
    DZItem::updateDbusConnections();
}

/*
 * Only needed if timeouts are used.
 */
extern "C" void taskTick(void)
{
    VeItem* veRoot = veValueTree();
    veItemTick(veRoot);
}

/*
 * Program version
 */
extern "C" char const *pltProgramVersion(void)
{
    return "dev";
}
