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

class DZValue
{
  public:
    static void onNotification(const Notification* _notification, void* _context);
    static size_t getVeItemDescription(VeItem* veItem, char* buf, size_t len);

    DZValue(uint32 zwaveHomeId, uint8 zwaveNodeId, ValueID zwaveValueId);

  private:
    static pthread_mutex_t        criticalSection;
    static map<VeItem*, DZValue*> veItemValueMapping;

    uint32                        zwaveHomeId;
    uint8                         zwaveNodeId;
    uint64                        zwaveValueId;
    VeItem*                       veItem;
    VeVariantUnitFmt*             veFmt;
    string                        description;

    ~DZValue();
    void onNotification(const Notification* _notification);
    void publish(ValueID zwaveValueId);
    void update(ValueID zwaveValueId);
};
