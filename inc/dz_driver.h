#ifndef _DZ_DRIVER_H
#define _DZ_DRIVER_H

#include <string>

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>

#include "dz_item.h"

using OpenZWave::Notification;
using std::string;

class DZDriver : protected DZItem
{
  public:
    DZDriver(uint32 zwaveHomeId);

    virtual void publish() override;
    virtual string getPath() override;

    void addNode();

  protected:
    uint32 zwaveHomeId;

    virtual void onZwaveNotification(const Notification* _notification) override;
    virtual void onVeItemChanged() override;

  private:
    static pthread_mutex_t  criticalSection;
    static volatile bool    initCompleted;
};

#endif