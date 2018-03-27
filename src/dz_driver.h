#ifndef _DZ_DRIVER_H
#define _DZ_DRIVER_H

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>

#include "dz_item.h"

using OpenZWave::Notification;
using std::string;

class DZDriver : DZItem
{
  public:
    static void changeVeValue(struct VeItem* item);

    DZDriver(uint32 zwaveHomeId);

  private:
    uint32                          zwaveHomeId;

    string getPath() override;
    void onNotification(const Notification* _notification) override;

    void publish();
    void addNode();
};

#endif
