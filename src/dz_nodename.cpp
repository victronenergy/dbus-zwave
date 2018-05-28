#include <string>

extern "C" {
#include <velib/types/variant.h>
#include <velib/types/ve_item_def.h>
#include <velib/utils/ve_item_utils.h>
#include <velib/utils/ve_logger.h>
}

#include <Defs.h>
#include <Manager.h>
#include <Notification.h>

#include "dz_nodename.hpp"
#include "dz_item.hpp"

using OpenZWave::Manager;
using OpenZWave::Notification;
using std::string;

static VeVariantUnitFmt unit = {0, ""};

DZNodeName::DZNodeName(uint32 zwaveHomeId)
{
    this->zwaveHomeId = zwaveHomeId;
    this->zwaveNodeId = Manager::Get()->GetControllerNodeId(zwaveHomeId);;
    this->path = DZItem::path(zwaveHomeId) + "/CustomName";
    this->serviceName = DZItem::getServiceName();
}

DZNodeName::DZNodeName(uint32 zwaveHomeId, uint8 zwaveNodeId)
{
    this->zwaveHomeId = zwaveHomeId;
    this->zwaveNodeId = zwaveNodeId;
    this->path = DZItem::path(zwaveHomeId, zwaveNodeId) + "/CustomName";
    this->serviceName = DZItem::getServiceName();
}

DZNodeName::DZNodeName(uint32 zwaveHomeId, string serviceName, string path) : DZNodeName(zwaveHomeId)
{
    this->path = path;
    this->serviceName = serviceName;
}

DZNodeName::DZNodeName(uint32 zwaveHomeId, uint8 zwaveNodeId, string serviceName, string path) : DZNodeName(zwaveHomeId, zwaveNodeId)
{
    this->path = path;
    this->serviceName = serviceName;
}

void DZNodeName::publish()
{
    this->description = "User editable name for a node";
    this->veFmt = &unit;

    DZItem::publish();

    VeVariant veVariant;
    veItemOwnerSet(this->veItem, veVariantHeapStr(&veVariant, Manager::Get()->GetNodeName(this->zwaveHomeId, this->zwaveNodeId).c_str()));
}

string DZNodeName::getServiceName()
{
    return this->serviceName;
}

string DZNodeName::getPath()
{
    return this->path;
}

void DZNodeName::onZwaveNotification(const Notification* _notification)
{
    if (
        _notification->GetHomeId() == this->zwaveHomeId &&
        _notification->GetNodeId() == this->zwaveNodeId &&
        _notification->GetType() == Notification::Type_NodeNaming
    ) {
        VeVariant veVariant;
        veItemOwnerSet(this->veItem, veVariantHeapStr(&veVariant, Manager::Get()->GetNodeName(this->zwaveHomeId, this->zwaveNodeId).c_str()));
    }
}

void DZNodeName::onVeItemChanged() {
    if (this->veItem->variant.type.tp != VE_HEAP_STR)
    {
        // TODO: return type error
        logE("DZNodeName", "Received invalid item change. Should be %d, got %d", VE_HEAP_STR, this->veItem->variant.type.tp);
        return;
    }

    string currentValue = Manager::Get()->GetNodeName(this->zwaveHomeId, this->zwaveNodeId);
    string newValue = string((char*) this->veItem->variant.value.CPtr);
    if (newValue != currentValue)
    {
        Manager::Get()->SetNodeName(this->zwaveHomeId, this->zwaveNodeId, newValue);
    }
}
