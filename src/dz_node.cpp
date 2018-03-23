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
#include "dz_node.h"

using OpenZWave::Manager;
using OpenZWave::Notification;
using std::map;
using std::string;

static VeVariantUnitFmt     unit = {0, ""};

map<VeItem*, DZNode*>       DZNode::veItemNodeMapping;
pthread_mutex_t             DZNode::criticalSection = [](){
    pthread_mutex_t criticalSection;
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&criticalSection, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
    return criticalSection;
}();

void DZNode::onNotification(const Notification* _notification, void* _context)
{
    return ((DZNode*) _context)->onNotification(_notification);
}

size_t DZNode::getVeItemDescription(VeItem* veItem, char* buf, size_t len)
{
    pthread_mutex_lock(&DZNode::criticalSection);
    size_t result = ve_snprintf(buf, len, "%s", DZNode::veItemNodeMapping[veItem]->description.c_str());
    pthread_mutex_unlock(&DZNode::criticalSection);
    return result;
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
    pthread_mutex_lock(&DZNode::criticalSection);
    DZNode::veItemNodeMapping.erase(this->veItem);
    pthread_mutex_unlock(&DZNode::criticalSection);
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
    pthread_mutex_lock(&DZNode::criticalSection);
    DZNode::veItemNodeMapping[this->veItem] = this;
    pthread_mutex_unlock(&DZNode::criticalSection);

    // Publish description
    this->description = Manager::Get()->GetNodeName(this->zwaveHomeId, this->zwaveNodeId);
    veItemSetGetDescr(this->veItem, &(DZNode::getVeItemDescription));
}
