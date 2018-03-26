#ifndef _DZ_NODE_H
#define _DZ_NODE_H

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>

#include "dz_item.h"

using OpenZWave::Notification;
using std::map;
using std::string;

class DZNode : DZItem
{
  public:
    static void onNotification(const Notification* _notification, void* _context);
    static void changeVeValue(struct VeItem* item);

    DZNode(uint32 zwaveHomeId, uint8 zwaveNodeId);

  private:
    static pthread_mutex_t        criticalSection;
    static map<VeItem*, DZNode*>  veItemNodeMapping;

    uint32                        zwaveHomeId;
    uint8                         zwaveNodeId;
    VeItem*                       veItem;
    VeVariantUnitFmt*             veFmt;

    ~DZNode();
    void onNotification(const Notification* _notification);
    void publish();
};

#endif
