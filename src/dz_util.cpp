#include <stdio.h>
#include <stdlib.h>
#include <map>

extern "C" {
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <value_classes/ValueID.h>

#include "dz_util.h"

using OpenZWave::ValueID;
using std::map;
using std::ostringstream;
using std::string;

string dz_path(uint32 zwaveHomeId)
{
    ostringstream path;
    path << "Zwave/" << +zwaveHomeId;
    return path.str();
}

string dz_path(uint32 zwaveHomeId, uint8 zwaveNodeId)
{
    ostringstream path;
    path << dz_path(zwaveHomeId) << "/" << +zwaveNodeId;
    return path.str();
}

string dz_path(uint32 zwaveHomeId, uint8 zwaveNodeId, ValueID zwaveValueId)
{
    ostringstream path;
    path << dz_path(zwaveHomeId, zwaveNodeId) << "/" << +zwaveValueId.GetCommandClassId() << "/" << +zwaveValueId.GetInstance() << "/" << +zwaveValueId.GetIndex();
    return path.str();
}

map<VeItem*, DZItem*> veDZItemMapping;
pthread_mutex_t       criticalSection = [](){
    pthread_mutex_t criticalSection;
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&criticalSection, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
    return criticalSection;
}();

void dz_itemmap_set(VeItem* veItem, DZItem* dzItem) {
    pthread_mutex_lock(&criticalSection);
    veDZItemMapping[veItem] = dzItem;
    pthread_mutex_unlock(&criticalSection);
}

DZItem* dz_itemmap_get(VeItem* veItem) {
    pthread_mutex_lock(&criticalSection);
    DZItem* result = veDZItemMapping[veItem];
    pthread_mutex_unlock(&criticalSection);
    return result;
}

void dz_itemmap_remove(VeItem* veItem) {
    pthread_mutex_lock(&criticalSection);
    veDZItemMapping.erase(veItem);
    pthread_mutex_unlock(&criticalSection);
}
