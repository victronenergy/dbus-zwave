#include <string>

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
#include <velib/types/ve_values.h>
#include <velib/utils/ve_item_utils.h>
#include <velib/utils/ve_logger.h>
#include <velib/vecan/products.h>
}

#include <Defs.h>
#include <Manager.h>
#include <Notification.h>

#include "dz_driver.hpp"
#include "dz_constvalue.hpp"
#include "dz_nodename.hpp"

using OpenZWave::Manager;
using OpenZWave::Notification;
using std::string;

static VeVariantUnitFmt unit = {0, ""};
volatile bool           DZDriver::initCompleted = false;
pthread_mutex_t         DZDriver::criticalSection = [](){
    pthread_mutex_t criticalSection;
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&criticalSection, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
    return criticalSection;
}();

DZDriver::DZDriver(uint32 zwaveHomeId)
{
    this->zwaveHomeId = zwaveHomeId;
}

void DZDriver::publish()
{
    this->description = "Controller";
    this->veFmt = &unit;

    uint8 controllerNodeId = Manager::Get()->GetControllerNodeId(this->zwaveHomeId);

    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/ProductName", Manager::Get()->GetNodeProductName(this->zwaveHomeId, controllerNodeId)));
    this->addAuxiliary(new DZNodeName(this->zwaveHomeId));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/ProductId", VE_PROD_NOT_SET));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/DeviceInstance", this->zwaveHomeId));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/Connected", true));
    //this->addAuxiliary(new DZSetting("Zwave/Test", 0));

    DZItem::publish();
}

string DZDriver::getPath()
{
    return DZItem::path(this->zwaveHomeId);
}

void DZDriver::onZwaveNotification(const Notification* _notification)
{
    if(_notification->GetHomeId() == this->zwaveHomeId)
    {
        switch (_notification->GetType())
        {
            case Notification::Type_DriverReset:
            {
                //TODO: Delete all underlying elements
            }

            case Notification::Type_AwakeNodesQueried:
            case Notification::Type_AllNodesQueried:
            case Notification::Type_AllNodesQueriedSomeDead:
            {
                pthread_mutex_lock(&DZDriver::criticalSection);
                Manager::Get()->WriteConfig(this->zwaveHomeId);
                DZDriver::initCompleted = true;
                pthread_mutex_unlock(&DZDriver::criticalSection);
                break;
            }

            case Notification::Type_NodeAdded:
            case Notification::Type_NodeNaming:
            case Notification::Type_NodeRemoved:
            case Notification::Type_ValueAdded:
            case Notification::Type_ValueRemoved:
            {
                this->writeConfig();
                break;
            }

            case Notification::Type_NodeQueriesComplete:
            {
                DZItem::connectServices();
                break;
            }

            case Notification::Type_ControllerCommand:
            {
                // Controller command completion follows adding a device,
                // connect any newly created services
                if (_notification->GetEvent() == 7)
                {
                    DZItem::connectServices();
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

void DZDriver::onVeItemChanged() {
    if (veVariantIsValid(&(this->veItem->variant)))
    {
        if (this->veItem->variant.type.tp == VE_SN32)
        {
            if (this->veItem->variant.value.SN32)
            {
                logI("DZDriver", "Mode set driver %d to add a node", +this->zwaveHomeId);
                Manager::Get()->AddNode(this->zwaveHomeId, true);
            }
            else
            {
                logI("DZDriver", "Mode set driver %d to remove a node", +this->zwaveHomeId);
                Manager::Get()->RemoveNode(this->zwaveHomeId);
            }
        }
        else
        {
            logE("DZDriver", "Received invalid set for driver. Should be %d, got %d", VE_UN8, this->veItem->variant.type.tp);
        }
    }

    // Reset value
    VeVariant veVariant;
    veVariantInvalidType(&veVariant, VE_SN32);
    veItemOwnerSet(this->veItem, &veVariant);
}

void DZDriver::writeConfig()
{
    pthread_mutex_lock(&DZDriver::criticalSection);
    if (DZDriver::initCompleted)
    {
        Manager::Get()->WriteConfig(this->zwaveHomeId);
    }
    pthread_mutex_unlock(&DZDriver::criticalSection);
}
