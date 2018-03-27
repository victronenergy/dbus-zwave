#ifndef _DZ_ITEM_H
#define _DZ_ITEM_H

#include <map>
#include <string>

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
using std::string;

/** Abstract base class for all items */
class DZItem
{
  public:
    static void onNotification(const Notification* _notification, void* _context);
    static size_t getVeItemDescription(VeItem* veItem, char* buf, size_t len);

  protected:
    static DZItem* get(VeItem* veItem);

    static string path(uint32 homeId);
    static string path(uint32 homeId, uint8 nodeId);
    static string path(uint32 homeId, uint8 nodeId, ValueID valueId);

    VeItem*           veItem;
    VeVariantUnitFmt* veFmt;
    string            description;

    void init();

    virtual ~DZItem();

    virtual VeItem* getRoot();
    virtual string getPath() = 0;

    virtual void onNotification(const Notification* _notification) = 0;

  private:
    static map<VeItem*, DZItem*> veDZItemMapping;
    static pthread_mutex_t       criticalSection;
};

#endif
