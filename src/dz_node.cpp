#include <string>

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
#include <velib/types/ve_values.h>
#include <velib/utils/ve_item_utils.h>
}

#include <Defs.h>
#include <Manager.h>
#include <Notification.h>

#include "dz_item.hpp"
#include "dz_node.hpp"

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
    this->description = Manager::Get()->GetNodeName(this->zwaveHomeId, this->zwaveNodeId);
    this->veFmt = &unit;
    DZItem::publish();
    this->description = Manager::Get()->GetNodeName(this->zwaveHomeId, this->zwaveNodeId);
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
            case Notification::Type_NodeNaming:
            {
                this->description = Manager::Get()->GetNodeName(this->zwaveHomeId, this->zwaveNodeId);
                break;
            }
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
