#include <string>

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
#include <velib/utils/ve_item_utils.h>
#include <velib/types/ve_values.h>
}

#include <Defs.h>
#include <Manager.h>
#include <Notification.h>

#include "dz_driver.h"

using OpenZWave::Manager;
using OpenZWave::Notification;
using std::string;

VeVariantUnitFmt unit = {0, ""};

void DZDriver::changeVeValue(VeItem* veItem)
{
    ((DZDriver*) DZItem::get(veItem))->addNode();
}

DZDriver::DZDriver(uint32 zwaveHomeId)
{
    this->zwaveHomeId = zwaveHomeId;
    this->description = "Controller";
    this->veFmt = &unit;

    this->init();

    // We abuse the change handler to trigger adding a node
    veItemSetChanged(this->veItem, &(DZDriver::changeVeValue));
}

string DZDriver::getPath()
{
    return DZItem::path(this->zwaveHomeId);
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

void DZDriver::addNode()
{
    Manager::Get()->AddNode(this->zwaveHomeId, true);
}
