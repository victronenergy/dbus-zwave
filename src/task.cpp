#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include <velib/platform/console.h>
#include <velib/platform/plt.h>
#include <velib/types/variant_print.h>
#include <velib/types/ve_dbus_item.h>
#include <velib/types/ve_item.h>
#include <velib/types/ve_item_def.h>
#include <velib/types/ve_values.h>
#include <velib/utils/ve_item_utils.h>
}

#include <Defs.h>
#include <Driver.h>
#include <Group.h>
#include <Manager.h>
#include <Node.h>
#include <Notification.h>
#include <Options.h>
#include <platform/Log.h>
#include <value_classes/Value.h>
#include <value_classes/ValueBool.h>
#include <value_classes/ValueID.h>
#include <value_classes/ValueStore.h>

using std::cerr;
using std::iterator;
using std::list;
using std::map;
using std::string;
using OpenZWave::ValueID;
using OpenZWave::Manager;
using OpenZWave::Options;
using OpenZWave::Notification;

typedef struct {
    uint32    zwaveHomeId;
    uint8     zwaveNodeId;
    ValueID*  zwaveValueId;
    VeItem*   veItem;
} PublishedItem;

static const string           defaultDriver = "/dev/ttyACM0";
static pthread_mutex_t        criticalSection;
static pthread_cond_t         initCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t        initMutex = PTHREAD_MUTEX_INITIALIZER;
static volatile bool          initFailed = false;
struct VeDbus*                dbusConnection;
static list<PublishedItem*>   publishedItems;

// 0 = ValueType_Bool        Boolean, true or false
// 1 = ValueType_Byte        8-bit unsigned value
// 2 = ValueType_Decimal     Represents a non-integer value as a string, to avoid floating point accuracy issues.
// 3 = ValueType_Int         32-bit signed value
// 4 = ValueType_List        List from which one item can be selected
// 5 = ValueType_Schedule    Complex type used with the Climate Control Schedule command class
// 6 = ValueType_Short       16-bit signed value
// 7 = ValueType_String      Text string
// 8 = ValueType_Button      A write-only value that is the equivalent of pressing a button to send a command to a device
// 9 = ValueType_Raw         A collection of bytes
static map<ValueID::ValueType, VeVariantUnitFmt> typeMapping = {
    {ValueID::ValueType_Bool, {0, " (boolean)"}},
    {ValueID::ValueType_Decimal, {0, " (decimal)"}},
    {ValueID::ValueType_String, {0, " (string)"}},
    {ValueID::ValueType_Byte, {0, " (byte)"}},
    {ValueID::ValueType_Short, {0, " (short)"}},
    {ValueID::ValueType_Int, {0, " (int)"}}
};

PublishedItem* getItem(uint32 zwaveHomeId, uint8 zwaveNodeId, ValueID zwaveValueId) {
    for(list<PublishedItem*>::iterator it = publishedItems.begin(); it != publishedItems.end(); it++) {
        PublishedItem* item = *it;
        if(item->zwaveHomeId == zwaveHomeId && item->zwaveNodeId == zwaveNodeId && *(item->zwaveValueId) == zwaveValueId) { // not sure if dereferencing is safe
            return item;
        }
    }
    return NULL;
}

void removeItem(uint32 zwaveHomeId, uint8 zwaveNodeId, ValueID zwaveValueId) {
    for(list<PublishedItem*>::iterator it = publishedItems.begin(); it != publishedItems.end(); it++) {
        PublishedItem* item = *it;
        if(item->zwaveHomeId == zwaveHomeId && item->zwaveNodeId == zwaveNodeId && *(item->zwaveValueId) == zwaveValueId) { // not sure if dereferencing is safe
            publishedItems.erase(it);
            // TODO: remove from dbus?
            delete item;
            break;
        }
    }
}

void updateItemValue(uint32 zwaveHomeId, uint8 zwaveNodeId, ValueID zwaveValueId) {
    VeVariant veVariant;
    PublishedItem* item = getItem(zwaveHomeId, zwaveNodeId, zwaveValueId);
    switch (zwaveValueId.GetType()) {
        case ValueID::ValueType_Bool:
        {
            bool value;
            Manager::Get()->GetValueAsBool(zwaveValueId, &value);
            veItemOwnerSet(item->veItem, veVariantBit(&veVariant, 1, value));
            break;
        }

        case ValueID::ValueType_Decimal:
        {
            string value;
            Manager::Get()->GetValueAsString(zwaveValueId, &value);
            // TODO use veVariantFloat ?
            veItemOwnerSet(item->veItem, veVariantHeapStr(&veVariant, value.c_str()));
            break;
        }

        case ValueID::ValueType_String:
        {
            string value;
            Manager::Get()->GetValueAsString(zwaveValueId, &value);
            veItemOwnerSet(item->veItem, veVariantHeapStr(&veVariant, value.c_str()));
            break;
        }

        case ValueID::ValueType_Byte:
        {
            uint8 value;
            Manager::Get()->GetValueAsByte(zwaveValueId, &value);
            veItemOwnerSet(item->veItem, veVariantUn8(&veVariant, +value));
            break;
        }

        case ValueID::ValueType_Short:
        {
            int16 value;
            Manager::Get()->GetValueAsShort(zwaveValueId, &value);
            veItemOwnerSet(item->veItem, veVariantSn16(&veVariant, +value));
            break;
        }

        case ValueID::ValueType_Int:
        {
            int32 value;
            Manager::Get()->GetValueAsInt(zwaveValueId, &value);
            veItemOwnerSet(item->veItem, veVariantSn32(&veVariant, +value));
            break;
        }

        default:
        {
            // TODO add list type using veVariantFloatArray ?
        }
    }
}

void addItem(uint32 zwaveHomeId, uint8 zwaveNodeId, ValueID zwaveValueId) {
    PublishedItem* item = new PublishedItem();
    item->zwaveHomeId = zwaveHomeId;
    item->zwaveNodeId = zwaveNodeId;
    item->zwaveValueId = &zwaveValueId;

    VeItem *veRoot = veValueTree();
	std::ostringstream path;
    path << "Zwave/" << +zwaveHomeId << "/" << +zwaveNodeId << "/" << Manager::Get()->GetValueLabel(zwaveValueId);
    veItemAddQuantity(veRoot, path.str().c_str(), item->veItem, &(typeMapping[zwaveValueId.GetType()]));

    publishedItems.push_back(item);

    // TODO implement unit
    // Manager::Get()->GetValueUnits(zwaveValueId)

    // TODO implement description string
    // Manager::Get()->GetValueHelp(zwaveValueId)

    // TODO implement min/max
    //VeVariant veVariant;
    //veItemSetMin(item->veItem, veVariantSn32(&veVariant, Manager::Get()->GetValueMin(zwaveValueId)));
    //veItemSetMax(item->veItem, veVariantSn32(&veVariant, Manager::Get()->GetValueMax(zwaveValueId)));

    updateItemValue(zwaveHomeId, zwaveNodeId, zwaveValueId);
}

void onZwaveNotification(Notification const* _notification, void* _context) {
    pthread_mutex_lock(&criticalSection);
    switch (_notification->GetType()) {
        case Notification::Type_ValueAdded:
        {
            addItem(_notification->GetHomeId(), _notification->GetNodeId(), _notification->GetValueID());
            break;
        }

        case Notification::Type_ValueChanged:
        {
            updateItemValue(_notification->GetHomeId(), _notification->GetNodeId(), _notification->GetValueID());
            break;
        }

        case Notification::Type_ValueRemoved:
        {
            removeItem(_notification->GetHomeId(), _notification->GetNodeId(), _notification->GetValueID());
            break;
        }

        case Notification::Type_Group:
        {
            // One of the node's association groups has changed
            break;
        }

        case Notification::Type_NodeAdded:
        {
            // Add the new node to our list
            // TODO: publish general info about device
            break;
        }

        case Notification::Type_NodeRemoved:
        {
            // Remove the node from our list
            // TODO: remove published general info about device
            break;
        }

        case Notification::Type_NodeEvent:
        {
            // Received an event from the node, caused by a basic_set or hail message
            break;
        }

        case Notification::Type_DriverFailed:
        {
            initFailed = true;
            pthread_cond_broadcast(&initCond);
            break;
        }

        case Notification::Type_AwakeNodesQueried:
        case Notification::Type_AllNodesQueried:
        case Notification::Type_AllNodesQueriedSomeDead:
        {
            pthread_cond_broadcast(&initCond);
            break;
        }

        case Notification::Type_PollingDisabled:
        case Notification::Type_PollingEnabled:
        case Notification::Type_DriverReady:
        case Notification::Type_DriverReset:
        case Notification::Type_Notification:
        case Notification::Type_NodeNaming:
        case Notification::Type_NodeProtocolInfo:
        case Notification::Type_NodeQueriesComplete:
        default:
        {
        }
    }
    pthread_mutex_unlock(&criticalSection);
}

extern "C" void taskInit(void)
{
    // Lock to prevent concurrent access to values tree
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&criticalSection, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
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
    if (initFailed) {
        cerr << "zwave connection failed\n";
        pltExit(1);
    }

    /* Connect to the dbus */
    if (! (dbusConnection = veDbusGetDefaultBus()) ) {
        cerr << "dbus connection failed\n";
        pltExit(5);
    }

    /* make the values also available on the dbus and get a service name */
    VeItem *veRoot = veValueTree();
    veDbusItemInit(dbusConnection, veRoot);

    if (!veDbusChangeName(dbusConnection, "com.victronenergy.zwave")) {
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
    VeItem *veRoot = veValueTree();
    veItemTick(veRoot);
}
