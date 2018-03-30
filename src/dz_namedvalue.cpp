#include <map>
#include <utility>
#include <tuple>

extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_dbus_item.h>
#include <velib/types/ve_item_def.h>
#include <velib/types/ve_values.h>
#include <velib/utils/ve_item_utils.h>
}

#include <Manager.h>
#include <value_classes/ValueID.h>

#include "dz_namedvalue.h"
#include "dz_value.h"

using OpenZWave::Manager;
using OpenZWave::ValueID;
using std::map;
using std::pair;
using std::string;
using std::tuple;

map<tuple<uint8, uint8, uint8>, pair<string, string>> DZNamedValue::namedValues;

bool DZNamedValue::isNamedValue(ValueID zwaveValueId)
{
    return DZNamedValue::namedValues.count(DZNamedValue::getValueSpec(zwaveValueId));
}

tuple<uint8, uint8, uint8> DZNamedValue::getValueSpec(ValueID zwaveValueId)
{
    DZNamedValue::namedValues[std::make_tuple(49, 1, 1)] = std::make_pair("com.victronenergy.temperature", "Temperature");
    return std::make_tuple(
        zwaveValueId.GetCommandClassId(),
        zwaveValueId.GetInstance(),
        zwaveValueId.GetIndex()
    );
}

DZNamedValue::DZNamedValue(ValueID zwaveValueId) : DZValue(zwaveValueId) {}

void DZNamedValue::publish()
{
    pair<string, string> name = DZNamedValue::namedValues[DZNamedValue::getValueSpec(this->zwaveValueId)];
    this->serviceName = name.first;
    this->path = name.second;
    DZValue::publish();
}

string DZNamedValue::getServiceName()
{
    return this->serviceName;
}

string DZNamedValue::getPath()
{
    return this->path;
}
