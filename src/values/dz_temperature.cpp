#include <string>

extern "C" {
#include <velib/platform/plt.h>
#include <velib/vecan/products.h>
}

#include <Manager.h>
#include <value_classes/ValueID.h>

#include "values/dz_temperature.hpp"
#include "dz_constvalue.hpp"
#include "dz_nodename.hpp"
#include "dz_value.hpp"

using OpenZWave::Manager;
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
    this->addAuxiliary(new DZConstValue(this->getServiceName(), "/ProductName",
        Manager::Get()->GetNodeProductName(this->zwaveValueId.GetHomeId(), this->zwaveValueId.GetNodeId())
    ));
    this->addAuxiliary(new DZNodeName(this->zwaveValueId.GetHomeId(), this->zwaveValueId.GetNodeId(), this->getServiceName(), "CustomName"));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), "Connected", true));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), "DeviceInstance", this->zwaveValueId.GetNodeId()));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), "ProductId", VE_PROD_NOT_SET));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), "Mgmt/ProcessName", pltProgramName()));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), "Mgmt/ProcessVersion", pltProgramVersion()));
    this->addAuxiliary(new DZConstValue(this->getServiceName(), "Mgmt/Connection", pltGetSerialDevice()));

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
