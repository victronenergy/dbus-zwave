#include <map>
#include <string>
#include <tuple>

#include <value_classes/ValueID.h>

#include "values/dz_gridmeter.hpp"
#include "dz_value.hpp"

using OpenZWave::ValueID;
using std::map;
using std::string;
using std::tuple;

map<tuple<uint8, uint8, uint8>, string> DZGridMeter::valueMapping = {
    {std::make_tuple(50, 1, 20), "AC/L1/Current"},
    {std::make_tuple(50, 1, 16), "AC/L1/Voltage"},
    {std::make_tuple(50, 1, 8), "AC/L1/Power"}
};

bool DZGridMeter::handles(ValueID zwaveValueId)
{
    return DZGridMeter::valueMapping.count(DZGridMeter::zwaveValueIdToTuple(zwaveValueId));
}

tuple<uint8, uint8, uint8> DZGridMeter::zwaveValueIdToTuple(ValueID zwaveValueId)
{
    return std::make_tuple(
        zwaveValueId.GetCommandClassId(),
        zwaveValueId.GetInstance(),
        zwaveValueId.GetIndex()
    );
}

DZGridMeter::DZGridMeter(ValueID zwaveValueId) : DZValue(zwaveValueId)
{}

void DZGridMeter::publish()
{
    this->path = DZGridMeter::valueMapping[DZGridMeter::zwaveValueIdToTuple(this->zwaveValueId)];

    DZValue::publish();
}

string DZGridMeter::getServiceName()
{
    return "com.victronenergy.grid";
}

string DZGridMeter::getPath()
{
    return this->path;
}
