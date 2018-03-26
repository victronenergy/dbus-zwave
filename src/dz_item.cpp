#include <string>

extern "C" {
#include <velib/base/ve_string.h>
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include "dz_item.h"
#include "dz_util.h"

using std::string;

size_t DZItem::getVeItemDescription(VeItem* veItem, char* buf, size_t len)
{
    return ve_snprintf(buf, len, "%s", dz_itemmap_get(veItem)->description.c_str());
}
