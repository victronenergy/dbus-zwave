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

#include "dz_item.hpp"
#include "dz_constvalue.hpp"
#include "dz_nodename.hpp"
#include "dz_setting.hpp"
#include "dz_util.hpp"

using OpenZWave::Manager;
using OpenZWave::Notification;
using OpenZWave::ValueID;
using std::map;
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

const string DZItem::defaultServiceName = "com.victronenergy.zwave";

void DZItem::connectServices()
{
    pthread_mutex_lock(&DZItem::criticalSection);
    for(const auto& service : DZItem::services)
    {
        if (service.second.first == NULL)
        {
            string serviceName = service.first;
            VeItem* veRoot = service.second.second;
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
                logE("DZItem", "dbus connection failed");
                pltExit(5);
            }

            // Register DBus service name
            if (!veDbusChangeName(dbusConnection, serviceName.c_str()))
            {
                logE("DZItem", "dbus_service: registering name %s failed", serviceName);
                pltExit(11);
            }

            // Register value tree onto the bus
            veDbusItemInit(dbusConnection, veRoot);

            // Update services list
            DZItem::services[serviceName] = std::make_pair(dbusConnection, veRoot);
        }
    }
    pthread_mutex_unlock(&DZItem::criticalSection);
}

void DZItem::updateDbusConnections()
{
    pthread_mutex_lock(&DZItem::criticalSection);
    for(const auto& service : DZItem::services)
    {
        if (service.second.first != NULL)
        {
            veDbusItemUpdate(service.second.first);
        }
    }
    pthread_mutex_unlock(&DZItem::criticalSection);
}

void DZItem::onZwaveNotification(const Notification* _notification, void* _context)
{
    return static_cast<DZItem*>(_context)->onZwaveNotification(_notification);
}

void DZItem::onVeItemChanged(VeItem* veItem)
{
    DZItem* dzItem = DZItem::get(veItem);
    if (dzItem != NULL)
    {
        dzItem->onVeItemChanged();

        // Run D-Bus change handler if D-Bus is connected
        if (DZItem::services[dzItem->getServiceName()].first != NULL)
        {
            dzItem->getServiceVeRoot()->changedFun(veItem); // onItemChanged
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

DZItem* DZItem::get(string serviceName, string path)
{
    DZItem* result = NULL;
    pthread_mutex_lock(&criticalSection);
    for (auto const& i : DZItem::veDZItemMapping)
    {
        DZItem* item = i.second;
        if (item->getServiceName() == serviceName && item->getPath() == path)
        {
            result = item;
            break;
        }
    }
    pthread_mutex_unlock(&criticalSection);
    return result;
}

DZItem* DZItem::get(uint32 zwaveHomeId)
{
    return DZItem::get(DZItem::defaultServiceName, DZUtil::path(zwaveHomeId));
}

DZItem* DZItem::get(uint32 zwaveHomeId, uint8 zwaveNodeId)
{
    return DZItem::get(DZItem::defaultServiceName, DZUtil::path(zwaveHomeId, zwaveNodeId));
}

DZItem* DZItem::get(uint32 zwaveHomeId, uint8 zwaveNodeId, uint8 zwaveCommandClassId)
{
    return DZItem::get(DZItem::defaultServiceName, DZUtil::path(zwaveHomeId, zwaveNodeId, zwaveCommandClassId));
}

DZItem* DZItem::get(ValueID zwaveValueId)
{
    return DZItem::get(DZItem::defaultServiceName, DZUtil::path(zwaveValueId));
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

DZItem::~DZItem()
{
    pthread_mutex_lock(&DZItem::criticalSection);
    bool veItemExists = veItemByUid(this->veItemPath.first, this->veItemPath.second.c_str()) != NULL;
    bool wasVeItemOwner = DZItem::veDZItemMapping[this->veItem] == this;
    if (wasVeItemOwner)
    {
        DZItem::veDZItemMapping.erase(this->veItem);
    }
    pthread_mutex_unlock(&DZItem::criticalSection);

    for(const auto& auxiliary : this->auxiliaries)
    {
        delete auxiliary;
    }
    this->auxiliaries.clear();

    Manager::Get()->RemoveWatcher(DZItem::onZwaveNotification, (void*) this);

    if (veItemExists && wasVeItemOwner)
    {
        pthread_mutex_lock(&DZItem::criticalSection);
        veItemDeleteBranch(this->veItem);
        pthread_mutex_unlock(&DZItem::criticalSection);
    }
}

void DZItem::publish()
{
    if (this->description.size())
    {
        logI("DZItem", "Publishing %s/%s: %s", this->getServiceName().c_str(), this->getPath().c_str(), this->description.c_str());
    }
    else
    {
        logI("DZItem", "Publishing %s/%s", this->getServiceName().c_str(), this->getPath().c_str());
    }

    pthread_mutex_lock(&DZItem::criticalSection);
    this->veItemPath = std::make_pair(this->getServiceVeRoot(), this->getPath());
    this->veItem = veItemGetOrCreateUid(this->veItemPath.first, this->veItemPath.second.c_str());
    pthread_mutex_unlock(&DZItem::criticalSection);

    this->veItem->changedFun = &DZItem::onVeItemChanged;

    veItemSetFmt(this->veItem, veVariantFmt, this->veFmt);

    pthread_mutex_lock(&DZItem::criticalSection);
    DZItem::veDZItemMapping[this->veItem] = this;
    pthread_mutex_unlock(&DZItem::criticalSection);

    veItemSetGetDescr(this->veItem, &DZItem::getVeItemDescription);

    Manager::Get()->AddWatcher(DZItem::onZwaveNotification, (void*) this);

    for(const auto& auxiliary : this->auxiliaries)
    {
        auxiliary->publish();
    }

    this->published = true;
}

string DZItem::getServiceName()
{
    return DZItem::defaultServiceName;
}

VeItem* DZItem::getServiceVeRoot()
{
    pthread_mutex_lock(&DZItem::criticalSection);
    string serviceName = this->getServiceName();
    if (!DZItem::services.count(serviceName))
    {
        DZItem::services[serviceName] = std::make_pair((VeDbus*) NULL, veItemGetOrCreateUid(veValueTree(), serviceName.c_str()));
    }
    VeItem* result = DZItem::services[serviceName].second;
    pthread_mutex_unlock(&DZItem::criticalSection);
    return result;
}

void DZItem::setServiceDbusConnection(VeDbus* dbusConnection)
{
    pthread_mutex_lock(&DZItem::criticalSection);
    string serviceName = this->getServiceName();
    VeItem* veRoot = this->getServiceVeRoot();
    DZItem::services[serviceName] = std::make_pair(dbusConnection, veRoot);
    pthread_mutex_unlock(&DZItem::criticalSection);
}

void DZItem::addAuxiliary(DZItem* item)
{
    pthread_mutex_lock(&DZItem::criticalSection);
    this->auxiliaries.insert(item);
    pthread_mutex_unlock(&DZItem::criticalSection);

    if (this->isPublished())
    {
        item->publish();
    }
}

void DZItem::addAuxiliary(DZNodeName* item)
{
    DZItem::addAuxiliary(static_cast<DZItem*>(item));
}
void DZItem::addAuxiliary(DZConstValue* item)
{
    DZItem::addAuxiliary(static_cast<DZItem*>(item));
}
void DZItem::addAuxiliary(DZSetting* item)
{
    DZItem::addAuxiliary(static_cast<DZItem*>(item));
}
