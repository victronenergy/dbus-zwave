#ifndef DZ_COMMANDCLASS_H
#define DZ_COMMANDCLASS_H

#include <string>

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>

#include "dz_item.hpp"

using OpenZWave::Notification;
using std::string;

class DZCommandClass : public DZItem
{
  public:
    explicit DZCommandClass(uint32 zwaveHomeId, uint8 zwaveNodeId, uint8 zwaveCommandClassId);

    virtual void publish() override;
    virtual string getPath() override;

  protected:
    uint32 zwaveHomeId;
    uint8  zwaveNodeId;
    uint8  zwaveCommandClassId;

    virtual void onZwaveNotification(const Notification* _notification) override;
    virtual void onVeItemChanged() override;
};

#endif
