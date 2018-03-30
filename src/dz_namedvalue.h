#ifndef _DZ_NAMEDVALUE_H
#define _DZ_NAMEDVALUE_H

#include <map>
#include <utility>
#include <tuple>

extern "C" {
#include <velib/types/ve_item_def.h>
}

#include <Defs.h>
#include <value_classes/ValueID.h>

#include "dz_value.h"

using OpenZWave::ValueID;
using std::map;
using std::pair;
using std::string;
using std::tuple;

class DZNamedValue : protected DZValue
{
  public:
    static bool isNamedValue(ValueID zwaveValueId);

    DZNamedValue(ValueID zwaveValueId);

    void publish() override;
    string getServiceName() override;
    string getPath() override;

  protected:
    static map<tuple<uint8, uint8, uint8>, pair<string, string>> namedValues;

    static tuple<uint8, uint8, uint8> getValueSpec(ValueID zwaveValueId);

    string serviceName;
    string path;
};

#endif
