#ifndef _DZ_VALUE_H
#define _DZ_VALUE_H

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <Notification.h>
#include <value_classes/ValueID.h>

#include "dz_item.h"

using OpenZWave::Notification;
using OpenZWave::ValueID;
using std::string;

class DZValue : protected DZItem
{
  public:
    DZValue(ValueID zwaveValueId);

    void publish() override;
    string getPath() override;

  protected:
    ValueID zwaveValueId;

    ~DZValue();

    void onNotification(const Notification* _notification) override;

    void update(ValueID zwaveValueId);
};

#endif
