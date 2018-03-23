extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>

using OpenZWave::Notification;
using std::map;
using std::string;

class DZDriver
{
  public:
    static void onNotification(const Notification* _notification, void* _context);
    static size_t getVeItemDescription(VeItem* veItem, char* buf, size_t len);
    static void changeVeValue(struct VeItem* item);

    DZDriver(uint32 zwaveHomeId);

  private:
    static pthread_mutex_t          criticalSection;
    static map<VeItem*, DZDriver*>  veItemDriverMapping;

    uint32                          zwaveHomeId;
    VeItem*                         veItem;
    VeVariantUnitFmt*               veFmt;
    string                          description;

    ~DZDriver();
    void onNotification(const Notification* _notification);
    void publish();
    void addNode();
};
