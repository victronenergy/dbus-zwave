#ifndef DZ_AEOTECZW095_H
#define DZ_AEOTECZW095_H

#include <map>
#include <tuple>

#include <Defs.h>
#include <value_classes/ValueID.h>

#include "dz_configurator.hpp"

using OpenZWave::ValueID;
using std::map;
using std::tuple;

class DZAeotecZw095 : public DZConfigurator
{
  public:
    static bool handles(ValueID zwaveValueId);

    explicit DZAeotecZw095(ValueID zwaveValueId);

    virtual void update() override;

  protected:
    static map<tuple<uint8, uint8, uint8>, int32> configMapping;
    static tuple<uint8, uint8, uint8> zwaveValueIdToTuple(ValueID zwaveValueId);

    int32 configValue;
};

#endif
