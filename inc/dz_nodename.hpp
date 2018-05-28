#ifndef DZ_NODENAME_H
#define DZ_NODENAME_H

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

class DZNodeName : public DZItem
{
  public:
    DZNodeName(uint32 zwaveHomeId);
    DZNodeName(uint32 zwaveHomeId, uint8 zwaveNodeId);

    virtual void publish() override;
    virtual string getPath() override;

  protected:
    uint32 zwaveHomeId;
    uint8  zwaveNodeId;
    bool   isController;

    virtual void onZwaveNotification(const Notification* _notification) override;
    virtual void onVeItemChanged() override;
};

#endif
