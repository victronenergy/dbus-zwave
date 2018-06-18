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

DZNode::DZNode(uint32 zwaveHomeId, uint8 zwaveNodeId)
{
    this->zwaveHomeId = zwaveHomeId;
    this->zwaveNodeId = zwaveNodeId;
}

void DZNode::publish()
{
    this->description = Manager::Get()->GetNodeType(this->zwaveHomeId, this->zwaveNodeId);
    this->veFmt = &veUnitNone;

    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/ProductName",
        Manager::Get()->GetNodeProductName(this->zwaveHomeId, this->zwaveNodeId)
    ));
    this->addAuxiliary(new DZNodeName(this->zwaveHomeId, this->zwaveNodeId));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/ProductId", VE_PROD_NOT_SET));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/DeviceInstance", this->zwaveNodeId));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/Connected",
        !Manager::Get()->IsNodeFailed(this->zwaveHomeId, this->zwaveNodeId))
    );

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

            case Notification::Type_Notification:
            {
                switch (_notification->GetNotification())
                {
                    case Notification::Code_Dead:
                    case Notification::Code_Alive:
                        (static_cast<DZConstValue*>(DZItem::get(this->getServiceName(), this->getPath() + "/Connected"))->set(
                            !Manager::Get()->IsNodeFailed(this->zwaveHomeId, this->zwaveNodeId))
                        );
                        break;

                    default:
                    {
                        break;
                    }
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }
}

void DZNode::onVeItemChanged()
{
    if (Manager::Get()->IsNodeFailed(this->zwaveHomeId, this->zwaveNodeId))
    {
        Manager::Get()->RemoveFailedNode(this->zwaveHomeId, this->zwaveNodeId);
    }
}
