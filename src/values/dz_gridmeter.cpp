#include <map>
#include <string>
#include <tuple>

extern "C" {
#include <velib/platform/plt.h>
#include <velib/vecan/products.h>
}

#include <Manager.h>
#include <value_classes/ValueID.h>

#include "values/dz_gridmeter.hpp"
#include "dz_constvalue.hpp"
#include "dz_nodename.hpp"
#include "dz_value.hpp"

using OpenZWave::Manager;
using OpenZWave::ValueID;
using std::map;
using std::string;
using std::tuple;

map<tuple<uint8, uint8, uint8>, string> DZGridMeter::valueMapping = {
    {std::make_tuple(50, 1, 20), "Ac/L1/Current"},
    {std::make_tuple(50, 1, 16), "Ac/L1/Voltage"},
    {std::make_tuple(50, 1, 8), "Ac/L1/Power"}
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

    DZValue::publish();
    this->updateTotals();
}

string DZGridMeter::getServiceName()
{
    return "com.victronenergy.grid";
}

string DZGridMeter::getPath()
{
    return this->path;
}

void DZGridMeter::onZwaveNotification(const Notification* _notification)
{
    DZValue::onZwaveNotification(_notification);
    if(_notification->GetValueID() == this->zwaveValueId && _notification->GetType() == Notification::Type_ValueChanged)
    {
        this->updateTotals();
    }
}

void DZGridMeter::updateTotals()
{
    // TODO: Check calculations

    // Current
    float totalCurrent = 0;
    for (const string &path : {"Ac/L1/Current", "Ac/L2/Current", "Ac/L3/Current"})
    {
        DZGridMeter* current = static_cast<DZGridMeter*>(DZItem::get(this->getServiceName(), path));
        if(current != NULL) {
            totalCurrent += current->veItem->variant.value.Float;
        }
    }

    DZConstValue* current = static_cast<DZConstValue*>(DZItem::get(this->getServiceName(), "Ac/Current"));
    if (current == NULL) {
        this->addAuxiliary(new DZConstValue(this->getServiceName(), "Ac/Current", totalCurrent, 0, "A"));
    } else {
        current->set(totalCurrent);
    }

    // Power
    float totalPower = 0;
    for (const string &path : {"Ac/L1/Power", "Ac/L2/Power", "Ac/L3/Power"})
    {
        DZGridMeter* power = static_cast<DZGridMeter*>(DZItem::get(this->getServiceName(), path));
        if(power != NULL) {
            totalPower += power->veItem->variant.value.Float;
        }
    }

    DZConstValue* power = static_cast<DZConstValue*>(DZItem::get(this->getServiceName(), "Ac/Power"));
    if (power == NULL) {
        this->addAuxiliary(new DZConstValue(this->getServiceName(), "Ac/Power", totalPower, 0, "W"));
    } else {
        power->set(totalPower);
    }
}
