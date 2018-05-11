#ifndef _DZ_TEMPERATURE_H
#define _DZ_TEMPERATURE_H

#include <string>

extern "C" {
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <value_classes/ValueID.h>

#include "dz_value.hpp"

using OpenZWave::ValueID;
using std::string;

class DZTemperature : protected DZValue
{
  public:
    static bool isTemperature(ValueID zwaveValueId);

    DZTemperature(ValueID zwaveValueId);

    virtual void publish() override;
    virtual string getServiceName() override;
    virtual string getPath() override;
};

#endif
