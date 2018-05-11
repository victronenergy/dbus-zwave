#ifndef DZ_NODE_H
#define DZ_NODE_H

#include <string>

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>

#include "dz_item.hpp"

using OpenZWave::Notification;
using std::map;
using std::string;

class DZNode : public DZItem
{
  public:
    DZNode(uint32 zwaveHomeId, uint8 zwaveNodeId);

    virtual void publish() override;
    virtual string getPath() override;

  protected:
    uint32                        zwaveHomeId;
    uint8                         zwaveNodeId;

    virtual void onZwaveNotification(const Notification* _notification) override;
    virtual void onVeItemChanged() override;
};

#endif
