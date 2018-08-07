#include <string>

extern "C" {
#include <velib/utils/ve_item_utils.h>
}

#include <Defs.h>
#include <Manager.h>
#include <Notification.h>

#include "dz_commandclass.hpp"
#include "dz_constvalue.hpp"
#include "dz_item.hpp"

using OpenZWave::Manager;
using OpenZWave::Notification;
using std::string;

DZCommandClass::DZCommandClass(uint32 zwaveHomeId, uint8 zwaveNodeId, uint8 zwaveCommandClassId)
{
    this->zwaveHomeId = zwaveHomeId;
    this->zwaveNodeId = zwaveNodeId;
    this->zwaveCommandClassId = zwaveCommandClassId;
}

void DZCommandClass::publish()
{
    this->description = "";
    this->veFmt = &veUnitNone;

    string name;
    uint8 version;
    Manager::Get()->GetNodeClassInformation(this->zwaveHomeId, this->zwaveNodeId, this->zwaveCommandClassId, &name, &version);
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/Name", name));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/Version", version));

    DZItem::publish();
}

string DZCommandClass::getPath()
{
    return DZItem::path(this->zwaveHomeId, this->zwaveNodeId, this->zwaveCommandClassId);
}

void DZCommandClass::onZwaveNotification(const Notification* _notification)
{
    if(
        _notification->GetHomeId() == this->zwaveHomeId &&
        _notification->GetNodeId() == this->zwaveNodeId &&
        _notification->GetType() == Notification::Type_NodeRemoved
    ) {
        delete this;
    }
}

void DZCommandClass::onVeItemChanged() {}
