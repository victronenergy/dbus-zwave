#ifndef _DZ_VALUE_H
#define _DZ_VALUE_H

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>
#include <value_classes/ValueID.h>

#include "dz_item.h"

using OpenZWave::Notification;
using OpenZWave::ValueID;
using std::string;

class DZValue : DZItem
{
  public:
    static void onNotification(const Notification* _notification, void* _context);

    DZValue(uint32 zwaveHomeId, uint8 zwaveNodeId, ValueID zwaveValueId);

  private:
    uint32                        zwaveHomeId;
    uint8                         zwaveNodeId;
    uint64                        zwaveValueId;
    VeItem*                       veItem;
    VeVariantUnitFmt*             veFmt;

    ~DZValue();
    void onNotification(const Notification* _notification);
    void publish(ValueID zwaveValueId);
    void update(ValueID zwaveValueId);
};

#endif
