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
#include <velib/utils/ve_logger.h>
#include <velib/utils/ve_assert.h>
}

#include <Manager.h>
#include <Notification.h>

#include "dz_item.h"
#include "dz_constvalue.h"

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
    for(const auto& service : DZItem::services)
    {
        veDbusItemUpdate(service.second.first);
    }
    pthread_mutex_unlock(&DZItem::criticalSection);
}

void DZItem::onZwaveNotification(const Notification* _notification, void* _context)
{
    return ((DZItem*) _context)->onZwaveNotification(_notification);
}

void DZItem::onVeItemChanged(VeItem* veItem)
{
    DZItem* dzItem = DZItem::get(veItem);
    if (dzItem != NULL)
    {
        dzItem->onVeItemChanged();
        if (dzItem->veItemChangedFun)
        {
            dzItem->veItemChangedFun(veItem);
        }
    }
}

size_t DZItem::getVeItemDescription(VeItem* veItem, char* buf, size_t len)
{
    DZItem* dzItem = DZItem::get(veItem);
    string description = "";
    if (dzItem != NULL)
    {
        description = DZItem::get(veItem)->description;
    }
    return ve_snprintf(buf, len, "%s", description.c_str());
}

DZItem* DZItem::get(VeItem* veItem)
{
    DZItem* result;
    pthread_mutex_lock(&criticalSection);
    if (DZItem::veDZItemMapping.count(veItem))
    {
        result = DZItem::veDZItemMapping.at(veItem);
    }
    else
    {
        result = NULL;
    }
    pthread_mutex_unlock(&criticalSection);
    return result;
}

string DZItem::path(uint32 zwaveHomeId)
{
    ostringstream path;
    path << "Interfaces/" << +zwaveHomeId;
    return path.str();
}

string DZItem::path(uint32 zwaveHomeId, uint8 zwaveNodeId)
{
    ostringstream path;
    path << DZItem::path(zwaveHomeId) << "/Devices/" << +zwaveNodeId;
    return path.str();
}

string DZItem::path(ValueID zwaveValueId)
{
    ostringstream path;
    path << DZItem::path(zwaveValueId.GetHomeId(), zwaveValueId.GetNodeId())
        << "/CommandClasses/" << +zwaveValueId.GetCommandClassId()
        << "/Instances/" << +zwaveValueId.GetInstance()
        << "/Indexes/" << +zwaveValueId.GetIndex();
    return path.str();
}

DZItem::~DZItem()
{

    pthread_mutex_lock(&DZItem::criticalSection);
    DZItem::veDZItemMapping.erase(this->veItem);
    pthread_mutex_unlock(&DZItem::criticalSection);

    for(const auto& auxiliary : this->auxiliaries)
    {
        delete auxiliary;
    }
    this->auxiliaries.clear();

    Manager::Get()->RemoveWatcher(DZItem::onZwaveNotification, (void*) this);

    veItemDeleteBranch(this->veItem);
}

void DZItem::publish()
{
    logI("task", "Publishing %s/%s: %s", this->getServiceName().c_str(), this->getPath().c_str(), this->description.c_str());

    pthread_mutex_lock(&DZItem::criticalSection);
    this->veItem = veItemGetOrCreateUid(this->getService().second, this->getPath().c_str());
    pthread_mutex_unlock(&DZItem::criticalSection);

    this->veItemChangedFun = this->veItem->changedFun;
    this->veItem->changedFun = &(DZItem::onVeItemChanged);

    veItemSetFmt(this->veItem, veVariantFmt, this->veFmt);

    pthread_mutex_lock(&DZItem::criticalSection);
    DZItem::veDZItemMapping[this->veItem] = this;
    pthread_mutex_unlock(&DZItem::criticalSection);

    veItemSetGetDescr(this->veItem, &(DZItem::getVeItemDescription));

    Manager::Get()->AddWatcher(DZItem::onZwaveNotification, (void*) this);

    for(const auto& auxiliary : this->auxiliaries)
    {
        auxiliary->publish();
    }
}

string DZItem::getServiceName()
{
    return "com.victronenergy.zwave";
}

pair<VeDbus*, VeItem*> DZItem::getService()
{
    pthread_mutex_lock(&DZItem::criticalSection);
    string serviceName = this->getServiceName();
    if (!DZItem::services.count(serviceName))
    {
        VeItem* veRoot = veItemGetOrCreateUid(veValueTree(), serviceName.c_str());
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
            logE("task", "dbus connection failed");
            pltExit(5);
        }

        // Register DBus service name
        if (!veDbusChangeName(dbusConnection, serviceName.c_str()))
        {
            logE("task", "dbus_service: registering name %s failed", serviceName);
            pltExit(11);
        }

        // Register value tree onto the bus
        veDbusItemInit(dbusConnection, veRoot);

        DZItem::services[serviceName] = std::make_pair(dbusConnection, veRoot);
    }
    pair<VeDbus*, VeItem*> result = DZItem::services[serviceName];
    pthread_mutex_unlock(&DZItem::criticalSection);
    return result;
}

void DZItem::setService(pair<VeDbus*, VeItem*> service)
{
    pthread_mutex_lock(&DZItem::criticalSection);
    string serviceName = this->getServiceName();
    veAssert(DZItem::services.count(serviceName) == 0);
    DZItem::services[serviceName] = service;
    pthread_mutex_unlock(&DZItem::criticalSection);
}

void DZItem::addAuxiliary(DZItem* item)
{
    pthread_mutex_lock(&DZItem::criticalSection);
    this->auxiliaries.insert(item);
    pthread_mutex_unlock(&DZItem::criticalSection);
}

void DZItem::addAuxiliary(DZConstValue* item)
{
    DZItem::addAuxiliary((DZItem*) item);
}
void DZItem::addAuxiliary(DZSetting* item)
{
    DZItem::addAuxiliary((DZItem*) item);
}
