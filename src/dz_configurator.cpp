#include <string>

extern "C" {
#include <velib/utils/ve_logger.h>
}

#include <Manager.h>
#include <Notification.h>
#include <value_classes/ValueID.h>

#include "dz_configurator.hpp"

using OpenZWave::Manager;
using OpenZWave::Notification;
using OpenZWave::ValueID;

void DZConfigurator::onZwaveNotification(const Notification* _notification, void* _context)
{
    return static_cast<DZConfigurator*>(_context)->onZwaveNotification(_notification);
}

DZConfigurator::DZConfigurator(ValueID zwaveValueId) : zwaveValueId(zwaveValueId)
{
    this->zwaveValueId = zwaveValueId;
}

void DZConfigurator::bind()
{
    logI("DZConfigurator", "Binding %d/%d/%d/%d/%d",
        +this->zwaveValueId.GetHomeId(),
        +this->zwaveValueId.GetNodeId(),
        +this->zwaveValueId.GetCommandClassId(),
        +this->zwaveValueId.GetInstance(),
        +this->zwaveValueId.GetIndex()
    );
    Manager::Get()->AddWatcher(DZConfigurator::onZwaveNotification, (void*) this);
    this->update();
}

DZConfigurator::~DZConfigurator()
{
    Manager::Get()->RemoveWatcher(DZConfigurator::onZwaveNotification, (void*) this);
}

void DZConfigurator::onZwaveNotification(const Notification* _notification)
{
    if(_notification->GetValueID() == this->zwaveValueId)
    {
        switch (_notification->GetType())
        {
            case Notification::Type_ValueChanged:
            {
                this->update();
                break;
            }

            case Notification::Type_ValueRemoved:
            {
                delete this;
                break;
            }

            default:
            {
                break;
            }
        }
    }
}
