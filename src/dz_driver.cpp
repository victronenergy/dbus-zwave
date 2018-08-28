#include <chrono>
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
#include "dz_util.hpp"

using OpenZWave::Manager;
using OpenZWave::Notification;
using std::chrono::duration_cast;
using std::chrono::seconds;
using std::chrono::system_clock;
using std::string;

DZDriver::DZDriver(uint32 zwaveHomeId)
{
    this->zwaveHomeId = zwaveHomeId;
    this->initCompleted = false;
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&this->criticalSection, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
}

DZDriver::~DZDriver()
{
    pthread_mutex_destroy(&this->criticalSection);
}

void DZDriver::publish()
{
    this->description = "Controller";
    this->veFmt = &veUnitNone;

    uint8 controllerNodeId = Manager::Get()->GetControllerNodeId(this->zwaveHomeId);

    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/ProductName", Manager::Get()->GetNodeProductName(this->zwaveHomeId, controllerNodeId)));
    this->addAuxiliary(new DZNodeName(this->zwaveHomeId));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/ProductId", VE_PROD_NOT_SET));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/DeviceInstance", this->zwaveHomeId));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), this->getPath() + "/Connected", true));
    //this->addAuxiliary(new DZSetting("Zwave/Test", 0));

    DZItem::publish();

    VeVariant veVariant;
    veItemOwnerSet(this->veItem, veVariantSn32(&veVariant, -1));
}

string DZDriver::getPath()
{
    return DZUtil::path(this->zwaveHomeId);
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
                pthread_mutex_lock(&this->criticalSection);
                this->initCompleted = true;
                this->writeConfig();
                pthread_mutex_unlock(&this->criticalSection);
                break;
            }

            case Notification::Type_NodeAdded:
            case Notification::Type_NodeNaming:
            case Notification::Type_NodeRemoved:
            case Notification::Type_ValueAdded:
            case Notification::Type_ValueRemoved:
            case Notification::Type_ValueChanged:
            {
                pthread_mutex_lock(&this->criticalSection);
                if (duration_cast<seconds>(system_clock::now() - this->lastWrite).count() > 30)
                {
                    this->writeConfig();
                }
                pthread_mutex_unlock(&this->criticalSection);
                break;
            }

            case Notification::Type_NodeQueriesComplete:
            {
                DZItem::connectServices();
                this->writeConfig();
                break;
            }

            case Notification::Type_ControllerCommand:
            {
                // Controller command completion follows adding a device,
                // connect any newly created services
                if (_notification->GetEvent() == 7)
                {
                    DZItem::connectServices();
                    this->writeConfig();
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

void DZDriver::onVeItemChanged()
{
    if (this->veItem->variant.type.tp == VE_SN32)
    {
        switch (this->veItem->variant.value.SN32)
        {
            case 1:
            {
                logI("DZDriver", "Mode set driver %d to add a node", +this->zwaveHomeId);
                Manager::Get()->AddNode(this->zwaveHomeId, true);
                break;
            }
            case 2:
            {
                logI("DZDriver", "Mode set driver %d to remove a node", +this->zwaveHomeId);
                Manager::Get()->RemoveNode(this->zwaveHomeId);
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else
    {
        logE("DZDriver", "Received invalid set for driver. Should be %d, got %d", VE_UN8, this->veItem->variant.type.tp);
    }

    // Reset value
    VeVariant veVariant;
    veItemOwnerSet(this->veItem, veVariantSn32(&veVariant, -1));
}

void DZDriver::writeConfig()
{
    pthread_mutex_lock(&this->criticalSection);
    if (this->initCompleted)
    {
        Manager::Get()->WriteConfig(this->zwaveHomeId);
        this->lastWrite = system_clock::now();
    }
    pthread_mutex_unlock(&this->criticalSection);
}
