#include <map>
#include <utility>
#include <string>
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

#include "values/dz_temperature.hpp"
#include "dz_value.hpp"
#include "dz_constvalue.hpp"

using OpenZWave::Manager;
using OpenZWave::ValueID;
using std::string;
using std::tuple;

bool DZTemperature::isTemperature(ValueID zwaveValueId)
{
    return
        zwaveValueId.GetCommandClassId() == 49
        &&
        zwaveValueId.GetInstance() == 1
        &&
        zwaveValueId.GetIndex() == 1;
}

DZTemperature::DZTemperature(ValueID zwaveValueId) : DZValue(zwaveValueId)
{}

void DZTemperature::publish()
{
    this->addAuxiliary(new DZConstValue(this->getServiceName(), "Status", 0));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), "TemperatureType", 2));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), "Scale", 1));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), "Offset", 0));

    DZValue::publish();
}

string DZTemperature::getServiceName()
{
    return "com.victronenergy.temperature";
}

string DZTemperature::getPath()
{
    return "Temperature";
}
