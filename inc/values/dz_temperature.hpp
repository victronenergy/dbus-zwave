#ifndef DZ_TEMPERATURE_H
#define DZ_TEMPERATURE_H

#include <string>

#include <value_classes/ValueID.h>

#include "dz_value.hpp"

using OpenZWave::ValueID;
using std::string;

class DZTemperature : public DZValue
{
  public:
    static bool handles(ValueID zwaveValueId);

    explicit DZTemperature(ValueID zwaveValueId);

    virtual void publish() override;
    virtual string getServiceName() override;
    virtual string getPath() override;
};

#endif
