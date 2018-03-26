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
#include "dz_util.h"
#include "dz_node.h"

using OpenZWave::Manager;
using OpenZWave::Notification;
using std::map;
using std::string;

static VeVariantUnitFmt     unit = {0, ""};

void DZNode::onNotification(const Notification* _notification, void* _context)
{
    return ((DZNode*) _context)->onNotification(_notification);
}

DZNode::DZNode(uint32 zwaveHomeId, uint8 zwaveNodeId)
{
    this->zwaveHomeId = zwaveHomeId;
    this->zwaveNodeId = zwaveNodeId;

    DZNode::publish();

    Manager::Get()->AddWatcher(DZNode::onNotification, (void*) this);
}

DZNode::~DZNode()
{
    Manager::Get()->RemoveWatcher(DZNode::onNotification, (void*) this);
    // TODO: remove from dbus?
    dz_itemmap_remove(this->veItem);
    delete this->veItem;
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

void DZNode::publish() {
    // Publish VeItem
    VeItem* veRoot = veValueTree();
    string path = dz_path(this->zwaveHomeId, this->zwaveNodeId);
    this->veItem = veItemGetOrCreateUid(veRoot, path.c_str());
    veItemSetFmt(this->veItem, veVariantFmt, &unit);

    // Create mapping
    dz_itemmap_set(this->veItem, this);

    // Publish description
    this->description = Manager::Get()->GetNodeName(this->zwaveHomeId, this->zwaveNodeId);
    veItemSetGetDescr(this->veItem, &(DZItem::getVeItemDescription));
}
