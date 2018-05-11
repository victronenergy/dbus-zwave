#ifndef _DZ_VALUE_H
#define _DZ_VALUE_H

#include <string>

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>
#include <value_classes/ValueID.h>

#include "dz_item.hpp"

using OpenZWave::Notification;
using OpenZWave::ValueID;
using std::string;

class DZValue : protected DZItem
{
  public:
    DZValue(ValueID zwaveValueId);

    virtual void publish() override;
    virtual string getPath() override;

  protected:
    ValueID zwaveValueId;

    ~DZValue();

    virtual void onZwaveNotification(const Notification* _notification) override;
    virtual void onVeItemChanged() override;

    void update(ValueID zwaveValueId);
};

#endif
