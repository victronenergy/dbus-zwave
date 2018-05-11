#include <string>

#include <value_classes/ValueID.h>

#include "values/dz_temperature_offset.hpp"
#include "dz_value.hpp"

using OpenZWave::ValueID;
using std::string;

bool DZTemperatureOffset::handles(ValueID zwaveValueId)
{
    return
        zwaveValueId.GetCommandClassId() == 112
        &&
        zwaveValueId.GetInstance() == 1
        &&
        zwaveValueId.GetIndex() == 66;
}

DZTemperatureOffset::DZTemperatureOffset(ValueID zwaveValueId) : DZValue(zwaveValueId)
{}

string DZTemperatureOffset::getServiceName()
{
    return "com.victronenergy.temperature";
}

string DZTemperatureOffset::getPath()
{
    return "Offset";
}
