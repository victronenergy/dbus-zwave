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

class DZNode : protected DZItem
{
  public:
    DZNode(uint32 zwaveHomeId, uint8 zwaveNodeId);

    void publish() override;
    string getPath() override;

  protected:
    uint32                        zwaveHomeId;
    uint8                         zwaveNodeId;

    void onNotification(const Notification* _notification) override;
};

#endif
