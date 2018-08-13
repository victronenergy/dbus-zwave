#ifndef DZ_AEOTECZW095_H
#define DZ_AEOTECZW095_H

#include <value_classes/ValueID.h>

#include "dz_configurator.hpp"

using OpenZWave::ValueID;

class DZAeotecZw095 : public DZConfigurator
{
  public:
    static bool handles(ValueID zwaveValueId) { return match(zwaveValueId, configMap, pollingMap); }
    explicit DZAeotecZw095(ValueID zwaveValueId) : DZConfigurator(zwaveValueId) {};

  protected:
    static ConfigValues configMap;
    static PollIntensities pollingMap;

    virtual ConfigValues    getConfigMap()  override { return configMap;  }
    virtual PollIntensities getPollingMap() override { return pollingMap; }
};

#endif
