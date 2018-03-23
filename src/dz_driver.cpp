extern "C" {
#include <velib/base/ve_string.h>
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
#include <velib/utils/ve_item_utils.h>
#include <velib/types/ve_values.h>
}

#include <Defs.h>
#include <Manager.h>
#include <Notification.h>

#include "dz_util.h"
#include "dz_driver.h"

using OpenZWave::Manager;
using OpenZWave::Notification;
using std::map;
using std::string;

static VeVariantUnitFmt     unit = {0, ""};

map<VeItem*, DZDriver*>     DZDriver::veItemDriverMapping;
pthread_mutex_t             DZDriver::criticalSection = [](){
    pthread_mutex_t criticalSection;
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&criticalSection, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
    return criticalSection;
}();

void DZDriver::onNotification(const Notification* _notification, void* _context)
{
    return ((DZDriver*) _context)->onNotification(_notification);
}

size_t DZDriver::getVeItemDescription(VeItem* veItem, char* buf, size_t len)
{
    pthread_mutex_lock(&DZDriver::criticalSection);
    size_t result = ve_snprintf(buf, len, "%s", DZDriver::veItemDriverMapping[veItem]->description.c_str());
    pthread_mutex_unlock(&DZDriver::criticalSection);
    return result;
}

void DZDriver::changeVeValue(VeItem* veItem)
{
    pthread_mutex_lock(&DZDriver::criticalSection);
    DZDriver::veItemDriverMapping[veItem]->addNode();
    pthread_mutex_unlock(&DZDriver::criticalSection);
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
    pthread_mutex_lock(&DZDriver::criticalSection);
    DZDriver::veItemDriverMapping.erase(this->veItem);
    pthread_mutex_unlock(&DZDriver::criticalSection);
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
    pthread_mutex_lock(&DZDriver::criticalSection);
    DZDriver::veItemDriverMapping[this->veItem] = this;
    pthread_mutex_unlock(&DZDriver::criticalSection);

    // Publish description
    this->description = "Controller";
    veItemSetGetDescr(this->veItem, &(DZDriver::getVeItemDescription));

    // Set change handler
    veItemSetChanged(this->veItem, &(DZDriver::changeVeValue));
}

void DZDriver::addNode()
{
    Manager::Get()->AddNode(this->zwaveHomeId, true);
}
