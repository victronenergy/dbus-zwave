extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
#include <velib/utils/ve_item_utils.h>
#include <velib/types/ve_values.h>
}

#include <Defs.h>
#include <Manager.h>
#include <Notification.h>

#include "dz_item.h"
#include "dz_node.h"

using OpenZWave::Manager;
using OpenZWave::Notification;
using std::map;
using std::string;

static VeVariantUnitFmt     unit = {0, ""};

DZNode::DZNode(uint32 zwaveHomeId, uint8 zwaveNodeId)
{
    this->zwaveHomeId = zwaveHomeId;
    this->zwaveNodeId = zwaveNodeId;
    this->description = Manager::Get()->GetNodeName(this->zwaveHomeId, this->zwaveNodeId);
    this->veFmt = &unit;

    this->init();
}

string DZNode::getPath()
{
    return DZItem::path(this->zwaveHomeId, this->zwaveNodeId);
}

void DZNode::onNotification(const Notification* _notification)
{
    if(_notification->GetHomeId() == this->zwaveHomeId)
    {
        switch (_notification->GetType())
        {
            case Notification::Type_NodeRemoved:
            {
                delete this;
            }

            default:
            {
            }
        }
    }
}
