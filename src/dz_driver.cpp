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
#include "dz_driver.h"

using OpenZWave::Manager;
using OpenZWave::Notification;
using std::map;
using std::string;

static VeVariantUnitFmt     unit = {0, ""};

void DZDriver::onNotification(const Notification* _notification, void* _context)
{
    return ((DZDriver*) _context)->onNotification(_notification);
}

void DZDriver::changeVeValue(VeItem* veItem)
{
    ((DZDriver*) dz_itemmap_get(veItem))->addNode();
}

DZDriver::DZDriver(uint32 zwaveHomeId)
{
    this->zwaveHomeId = zwaveHomeId;

    DZDriver::publish();

    Manager::Get()->AddWatcher(DZDriver::onNotification, (void*) this);
}

DZDriver::~DZDriver()
{
    Manager::Get()->RemoveWatcher(DZDriver::onNotification, (void*) this);
    // TODO: remove from dbus?
    dz_itemmap_remove(this->veItem);
    delete this->veItem;
}

void DZDriver::onNotification(const Notification* _notification)
{
    if(_notification->GetHomeId() == this->zwaveHomeId)
    {
        switch (_notification->GetType())
        {
            case Notification::Type_DriverReset:
            {
                //TODO: Delete all underlying elements
            }

            default:
            {
            }
        }
    }
}

void DZDriver::publish() {
    // Publish VeItem
    VeItem* veRoot = veValueTree();
    string path = dz_path(zwaveHomeId);
    this->veItem = veItemGetOrCreateUid(veRoot, path.c_str());
    veItemSetFmt(this->veItem, veVariantFmt, &unit);

    // Create mapping
    dz_itemmap_set(this->veItem, this);

    // Publish description
    this->description = "Controller";
    veItemSetGetDescr(this->veItem, &(DZItem::getVeItemDescription));

    // Set change handler
    veItemSetChanged(this->veItem, &(DZDriver::changeVeValue));
}

void DZDriver::addNode()
{
    Manager::Get()->AddNode(this->zwaveHomeId, true);
}
