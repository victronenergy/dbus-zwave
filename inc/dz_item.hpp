#ifndef DZ_ITEM_H
#define DZ_ITEM_H

#include <map>
#include <set>
#include <string>
#include <utility>

extern "C" {
#include <velib/types/variant.h>
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>
#include <value_classes/ValueID.h>

class DZNodeName;
class DZConstValue;
class DZSetting;

using OpenZWave::Notification;
using OpenZWave::ValueID;
using std::map;
using std::pair;
using std::set;
using std::string;

/** Abstract base class for all items */
class DZItem
{
  public:
    static void updateDbusConnections();
    static void onZwaveNotification(const Notification* _notification, void* _context);
    static void onVeItemChanged(VeItem* veItem);
    static size_t getVeItemDescription(VeItem* veItem, char* buf, size_t len);

    virtual void publish();
    virtual string getServiceName();
    virtual string getPath() = 0;

  protected:
    static DZItem* get(string serviceName, string path);
    static DZItem* get(VeItem* veItem);
    static string path(uint32 homeId);
    static string path(uint32 homeId, uint8 nodeId);
    static string path(ValueID valueId);

    VeItem*           veItem;
    VeVariantUnitFmt* veFmt;
    string            description;

    virtual ~DZItem();

    virtual void onZwaveNotification(const Notification* _notification) = 0;
    virtual void onVeItemChanged() = 0;

    bool isPublished() { return this->published; }
    pair<VeDbus*, VeItem*> getService();
    void setService(pair<VeDbus*, VeItem*> service);
    void addAuxiliary(DZNodeName* item);
    void addAuxiliary(DZConstValue* item);
    void addAuxiliary(DZSetting* item);

  private:
    static map<string, pair<VeDbus*, VeItem*>>  services;
    static map<VeItem*, DZItem*>                veDZItemMapping;
    static pthread_mutex_t                      criticalSection;

    bool                                        published = false;
    set<DZItem*>                                auxiliaries;
    VeItemValueChanged*                         veItemChangedFun;

    void addAuxiliary(DZItem* item);
};

#endif
