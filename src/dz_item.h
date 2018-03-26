#ifndef _DZ_ITEM_H
#define _DZ_ITEM_H

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

using std::string;

/** Abstract base class for all items */
class DZItem
{
  public:
    static size_t getVeItemDescription(VeItem* veItem, char* buf, size_t len);

  protected:
    string description;
};

#endif
