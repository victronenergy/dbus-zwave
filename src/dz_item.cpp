#include <map>
#include <string>
#include <utility>

extern "C" {
#include <velib/base/ve_string.h>
#include <velib/platform/plt.h>
#include <velib/types/variant_print.h>
#include <velib/types/ve_dbus_item.h>
#include <velib/types/ve_item.h>
#include <velib/types/ve_item_def.h>
#include <velib/types/ve_values.h>
}

#include <Manager.h>
#include <Notification.h>

#include "dz_item.h"

using OpenZWave::Manager;
using OpenZWave::Notification;
using OpenZWave::ValueID;
using std::map;
using std::ostringstream;
using std::pair;
using std::string;

map<string, pair<VeDbus*, VeItem*>> DZItem::services;
map<VeItem*, DZItem*>               DZItem::veDZItemMapping;
pthread_mutex_t                     DZItem::criticalSection = [](){
    pthread_mutex_t criticalSection;
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&criticalSection, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
    return criticalSection;
}();

void DZItem::updateDbusConnections()
{
    pthread_mutex_lock(&DZItem::criticalSection);
    for(const auto& service : DZItem::services) {
        veDbusItemUpdate(service.second.first);
    }
    pthread_mutex_unlock(&DZItem::criticalSection);
}

void DZItem::onNotification(const Notification* _notification, void* _context)
{
    return ((DZItem*) _context)->onNotification(_notification);
}

size_t DZItem::getVeItemDescription(VeItem* veItem, char* buf, size_t len)
{
    return ve_snprintf(buf, len, "%s", DZItem::get(veItem)->description.c_str());
}

DZItem* DZItem::get(VeItem* veItem) {
    pthread_mutex_lock(&criticalSection);
    DZItem* result = veDZItemMapping[veItem];
    pthread_mutex_unlock(&criticalSection);
    return result;
}
string DZItem::path(uint32 zwaveHomeId)
{
    ostringstream path;
    path << "Zwave/" << +zwaveHomeId;
    return path.str();
}

string DZItem::path(uint32 zwaveHomeId, uint8 zwaveNodeId)
{
    ostringstream path;
    path << DZItem::path(zwaveHomeId) << "/" << +zwaveNodeId;
    return path.str();
}

string DZItem::path(ValueID zwaveValueId)
{
    ostringstream path;
    path << DZItem::path(zwaveValueId.GetHomeId(), zwaveValueId.GetNodeId()) << "/" << +zwaveValueId.GetCommandClassId() << "/" << +zwaveValueId.GetInstance() << "/" << +zwaveValueId.GetIndex();
    return path.str();
}

DZItem::~DZItem()
{
    Manager::Get()->RemoveWatcher(DZItem::onNotification, (void*) this);

    pthread_mutex_lock(&DZItem::criticalSection);
    DZItem::veDZItemMapping.erase(this->veItem);
    pthread_mutex_unlock(&DZItem::criticalSection);

    veItemDeleteBranch(this->veItem);
}

void DZItem::publish() {
    pthread_mutex_lock(&DZItem::criticalSection);
    string serviceName = this->getServiceName();
    if (!DZItem::services.count(serviceName))
    {
        VeDbus* dbusConnection;

        // Use default bus for unnamed items
        if (serviceName == "com.victronenergy.zwave")
        {
            dbusConnection = veDbusGetDefaultBus();
        }
        else
        {
            dbusConnection = veDbusConnectString(veDbusGetDefaultConnectString());
        }

        // DBus failures are fatal
        if (!dbusConnection)
        {
            std::cerr << "dbus connection failed\n";
            pltExit(5);
        }

        if (!veDbusChangeName(dbusConnection, serviceName.c_str()))
        {
            std::cerr << "dbus_service: registering name " << serviceName << " failed\n";
            pltExit(11);
        }

        // Register value tree onto the bus
        VeItem* veRoot = veItemGetOrCreateUid(veValueTree(), serviceName.c_str());
        veDbusItemInit(dbusConnection, veRoot);

        DZItem::services[serviceName] = std::make_pair(dbusConnection, veRoot);
    }

#ifdef DEBUG
    std::cerr << "Publishing " << serviceName << "/" << this->getPath() << "\n";
#endif

    this->veItem = veItemGetOrCreateUid(DZItem::services[serviceName].second, this->getPath().c_str());
    pthread_mutex_unlock(&DZItem::criticalSection);
    veItemSetFmt(this->veItem, veVariantFmt, this->veFmt);

    pthread_mutex_lock(&criticalSection);
    veDZItemMapping[this->veItem] = this;
    pthread_mutex_unlock(&criticalSection);

    veItemSetGetDescr(this->veItem, &(DZItem::getVeItemDescription));

    Manager::Get()->AddWatcher(DZItem::onNotification, (void*) this);
}

string DZItem::getServiceName()
{
    return "com.victronenergy.zwave";
}
