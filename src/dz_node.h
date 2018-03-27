#ifndef _DZ_NODE_H
#define _DZ_NODE_H

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>

#include "dz_item.h"

using OpenZWave::Notification;
using std::map;
using std::string;

class DZNode : DZItem
{
  public:
    DZNode(uint32 zwaveHomeId, uint8 zwaveNodeId);

  private:
    uint32                        zwaveHomeId;
    uint8                         zwaveNodeId;

    string getPath() override;
    void onNotification(const Notification* _notification) override;
};

#endif
