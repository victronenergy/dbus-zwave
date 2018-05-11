#include <string>

#include <value_classes/ValueID.h>

#include "values/dz_temperature.hpp"
#include "dz_value.hpp"
#include "dz_constvalue.hpp"

using OpenZWave::ValueID;
using std::string;

bool DZTemperature::handles(ValueID zwaveValueId)
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
    if (DZItem::get(this->getServiceName(), "Offset") == NULL) {
        this->addAuxiliary(new DZConstValue(this->getServiceName(), "Offset", 0));
    }

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
