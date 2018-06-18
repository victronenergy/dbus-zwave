#ifndef DZ_DRIVER_H
#define DZ_DRIVER_H

#include <chrono>
#include <string>

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>

#include "dz_item.hpp"

using OpenZWave::Notification;
using std::chrono::system_clock;
using std::string;

class DZDriver : public DZItem
{
  public:
    explicit DZDriver(uint32 zwaveHomeId);
    virtual ~DZDriver();

    virtual void publish() override;
    virtual string getPath() override;

    void writeConfig();

  protected:
    uint32 zwaveHomeId;

    virtual void onZwaveNotification(const Notification* _notification) override;
    virtual void onVeItemChanged() override;

  private:
    pthread_mutex_t            criticalSection;
    volatile bool              initCompleted;
    system_clock::time_point   lastWrite;
};

#endif
