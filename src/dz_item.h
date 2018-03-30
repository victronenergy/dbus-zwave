#ifndef _DZ_ITEM_H
#define _DZ_ITEM_H

#include <map>
#include <string>
#include <utility>

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>
#include <value_classes/ValueID.h>

using OpenZWave::Notification;
using OpenZWave::ValueID;
using std::map;
using std::pair;
using std::string;

/** Abstract base class for all items */
class DZItem
{
  public:
    static void updateDbusConnections();
    static void onNotification(const Notification* _notification, void* _context);
    static size_t getVeItemDescription(VeItem* veItem, char* buf, size_t len);

    virtual void publish();
    virtual string getServiceName();
    virtual string getPath() = 0;

  protected:

    static DZItem* get(VeItem* veItem);

    static string path(uint32 homeId);
    static string path(uint32 homeId, uint8 nodeId);
    static string path(ValueID valueId);

    VeItem*           veItem;
    VeVariantUnitFmt* veFmt;
    string            description;

    virtual ~DZItem();

    virtual void onNotification(const Notification* _notification) = 0;

  private:
    static map<string, pair<VeDbus*, VeItem*>>  services;
    static map<VeItem*, DZItem*>                veDZItemMapping;
    static pthread_mutex_t                      criticalSection;
};

#endif
