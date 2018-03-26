#ifndef _DZ_UTIL_H
#define _DZ_UTIL_H

#include <map>

#include <Defs.h>
#include <value_classes/ValueID.h>

#include "dz_item.h"

using OpenZWave::ValueID;
using std::map;
using std::string;

string dz_path(uint32 homeId);
string dz_path(uint32 homeId, uint8 nodeId);
string dz_path(uint32 homeId, uint8 nodeId, ValueID valueId);

void dz_itemmap_set(VeItem* veItem, DZItem* dzItem);
DZItem* dz_itemmap_get(VeItem* veItem);
void dz_itemmap_remove(VeItem* veItem);

#endif
