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
    explicit DZNodeName(uint32 zwaveHomeId);
    explicit DZNodeName(uint32 zwaveHomeId, uint8 zwaveNodeId);
    explicit DZNodeName(uint32 zwaveHomeId, string serviceName, string path);
    explicit DZNodeName(uint32 zwaveHomeId, uint8 zwaveNodeId, string serviceName, string path);

    virtual void publish() override;
    virtual string getServiceName() override;
    virtual string getPath() override;

  protected:
    uint32 zwaveHomeId;
    uint8  zwaveNodeId;
    string serviceName;
    string path;

    virtual void onZwaveNotification(const Notification* _notification) override;
    virtual void onVeItemChanged() override;
};

#endif
