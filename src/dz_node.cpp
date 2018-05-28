#include <string>

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
#include <velib/types/ve_values.h>
#include <velib/utils/ve_item_utils.h>
#include <velib/vecan/products.h>
}

#include <Defs.h>
#include <Manager.h>
#include <Notification.h>

#include "dz_node.hpp"
#include "dz_constvalue.hpp"
#include "dz_item.hpp"
#include "dz_nodename.hpp"

using OpenZWave::Manager;
using OpenZWave::Notification;
using std::string;

static VeVariantUnitFmt     unit = {0, ""};

DZNode::DZNode(uint32 zwaveHomeId, uint8 zwaveNodeId)
{
    this->zwaveHomeId = zwaveHomeId;
    this->zwaveNodeId = zwaveNodeId;
}

void DZNode::publish()
{
    this->description = Manager::Get()->GetNodeType(this->zwaveHomeId, this->zwaveNodeId);
    this->veFmt = &unit;

    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/ProductName",
        Manager::Get()->GetNodeProductName(this->zwaveHomeId, this->zwaveNodeId)
    ));
    this->addAuxiliary(new DZNodeName(this->zwaveHomeId, this->zwaveNodeId));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/ProductId", VE_PROD_NOT_SET));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/DeviceInstance", this->zwaveNodeId));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/Connected", true));

    DZItem::publish();
}

string DZNode::getPath()
{
    return DZItem::path(this->zwaveHomeId, this->zwaveNodeId);
}

void DZNode::onZwaveNotification(const Notification* _notification)
{
    if(_notification->GetHomeId() == this->zwaveHomeId && _notification->GetNodeId() == this->zwaveNodeId)
    {
        switch (_notification->GetType())
        {
            case Notification::Type_NodeRemoved:
            {
                delete this;
                break;
            }

            default:
            {
            }
        }
    }
}

void DZNode::onVeItemChanged() {}
