#ifndef DZ_TEMPERATURE_OFFSET_H
#define DZ_TEMPERATURE_OFFSET_H

#include <string>

#include <value_classes/ValueID.h>

#include "dz_value.hpp"

using OpenZWave::ValueID;
using std::string;

class DZTemperatureOffset : public DZValue
{
  public:
    static bool handles(ValueID zwaveValueId);

    DZTemperatureOffset(ValueID zwaveValueId);

    virtual string getServiceName() override;
    virtual string getPath() override;
};

#endif
