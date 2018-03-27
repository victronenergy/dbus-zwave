#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include <velib/platform/console.h>
#include <velib/platform/plt.h>
#include <velib/types/ve_dbus_item.h>
#include <velib/types/ve_item.h>
#include <velib/types/ve_values.h>
}

#include <Defs.h>
#include <Driver.h>
#include <Manager.h>
#include <Notification.h>
#include <Options.h>
#include <value_classes/ValueID.h>

#include "dz_driver.h"
#include "dz_node.h"
#include "dz_value.h"

using OpenZWave::Manager;
using OpenZWave::Notification;
using OpenZWave::Options;
using OpenZWave::ValueID;
using std::cerr;

static const string           defaultDriver = "/dev/ttyS8";
static pthread_cond_t         initCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t        initMutex = PTHREAD_MUTEX_INITIALIZER;
static volatile bool          initFailed = false;
struct VeDbus*                dbusConnection;

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
            new DZDriver(_notification->GetHomeId());
            break;
        }

        case Notification::Type_NodeAdded:
        {
            new DZNode(_notification->GetHomeId(), _notification->GetNodeId());
            break;
        }

        case Notification::Type_ValueAdded:
        {
            new DZValue(_notification->GetHomeId(), _notification->GetNodeId(), _notification->GetValueID());
            break;
        }

        default:
        {
        }
    }
}

extern "C" void taskInit(void)
{
    pthread_mutex_lock(&initMutex);

    Options::Create("config/", "", "");
    Options::Get()->AddOptionBool("ConsoleOutput", false);
    Options::Get()->Lock();

    Manager::Create();
    Manager::Get()->AddWatcher(onZwaveNotification, NULL);

    // Add drivers
    Manager::Get()->AddDriver(defaultDriver);

    // Wait for successful init
    pthread_cond_wait(&initCond, &initMutex);
    if (initFailed)
    {
        cerr << "zwave connection failed\n";
        pltExit(1);
    }

    /* Connect to the dbus */
    if (!(dbusConnection = veDbusGetDefaultBus()))
    {
        cerr << "dbus connection failed\n";
        pltExit(5);
    }

    /* make the values also available on the dbus and get a service name */
    VeItem* veRoot = veValueTree();
    veDbusItemInit(dbusConnection, veRoot);

    if (!veDbusChangeName(dbusConnection, "com.victronenergy.zwave"))
    {
        cerr << "dbus_service: registering name failed\n";
        pltExit(11);
    }
}

/*
 * Note: only needed for the blocking version.
 * not needed when using libevent.
 */
extern "C" void taskUpdate(void)
{
    if (dbusConnection)
        veDbusItemUpdate(dbusConnection);
}

/*
 * Only needed if timeouts are used.
 */
extern "C" void taskTick(void)
{
    VeItem* veRoot = veValueTree();
    veItemTick(veRoot);
}
