#include <map>
#include <string>

extern "C" {
#include <velib/base/ve_string.h>
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
#include <velib/types/ve_values.h>
#include <velib/types/ve_item.h>
}

#include <Manager.h>
#include <Notification.h>

#include "dz_item.h"

using OpenZWave::Manager;
using OpenZWave::Notification;
using OpenZWave::ValueID;
using std::ostringstream;
using std::string;

map<VeItem*, DZItem*> DZItem::veDZItemMapping;
pthread_mutex_t       DZItem::criticalSection = [](){
    pthread_mutex_t criticalSection;
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&criticalSection, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
    return criticalSection;
}();

void DZItem::onNotification(const Notification* _notification, void* _context)
{
    return ((DZItem*) _context)->onNotification(_notification);
}

size_t DZItem::getVeItemDescription(VeItem* veItem, char* buf, size_t len)
{
    return ve_snprintf(buf, len, "%s", DZItem::get(veItem)->description.c_str());
}

DZItem* DZItem::get(VeItem* veItem) {
    pthread_mutex_lock(&criticalSection);
    DZItem* result = veDZItemMapping[veItem];
    pthread_mutex_unlock(&criticalSection);
    return result;
}
string DZItem::path(uint32 zwaveHomeId)
{
    ostringstream path;
    path << "Zwave/" << +zwaveHomeId;
    return path.str();
}

string DZItem::path(uint32 zwaveHomeId, uint8 zwaveNodeId)
{
    ostringstream path;
    path << DZItem::path(zwaveHomeId) << "/" << +zwaveNodeId;
    return path.str();
}

string DZItem::path(uint32 zwaveHomeId, uint8 zwaveNodeId, ValueID zwaveValueId)
{
    ostringstream path;
    path << DZItem::path(zwaveHomeId, zwaveNodeId) << "/" << +zwaveValueId.GetCommandClassId() << "/" << +zwaveValueId.GetInstance() << "/" << +zwaveValueId.GetIndex();
    return path.str();
}

DZItem::~DZItem()
{
    Manager::Get()->RemoveWatcher(DZItem::onNotification, (void*) this);

    pthread_mutex_lock(&criticalSection);
    veDZItemMapping.erase(this->veItem);
    pthread_mutex_unlock(&criticalSection);

    veItemDeleteBranch(this->veItem);
}

void DZItem::init() {
    this->veItem = veItemGetOrCreateUid(this->getRoot(), this->getPath().c_str());
    veItemSetFmt(this->veItem, veVariantFmt, this->veFmt);

    pthread_mutex_lock(&criticalSection);
    veDZItemMapping[this->veItem] = this;
    pthread_mutex_unlock(&criticalSection);

    veItemSetGetDescr(this->veItem, &(DZItem::getVeItemDescription));

    Manager::Get()->AddWatcher(DZItem::onNotification, (void*) this);
}

VeItem* DZItem::getRoot()
{
    return veValueTree();
}
