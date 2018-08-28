#include <map>
#include <string>
#include <tuple>

extern "C" {
#include <velib/platform/plt.h>
#include <velib/vecan/products.h>
}

#include <Defs.h>
#include <Manager.h>
#include <value_classes/ValueID.h>

#include "values/dz_gridmeter.hpp"
#include "dz_constvalue.hpp"
#include "dz_nodename.hpp"
#include "dz_util.hpp"
#include "dz_value.hpp"

using OpenZWave::Manager;
using OpenZWave::ValueID;
using std::map;
using std::string;
using std::tuple;

map<DZUtil::MatchSpec, string> DZGridMeter::valueMapping = {
    {{{}, {}, {}, {50}, {1}, {45}}, "Ac/L1/Current"},
    {{{}, {}, {}, {50}, {1}, {36}}, "Ac/L1/Voltage"},
    {{{}, {}, {}, {50}, {1}, {18}}, "Ac/L1/Power"},
    {{{}, {}, {}, {50}, {2}, {45}}, "Ac/L2/Current"},
    {{{}, {}, {}, {50}, {2}, {36}}, "Ac/L2/Voltage"},
    {{{}, {}, {}, {50}, {2}, {18}}, "Ac/L2/Power"},
    {{{}, {}, {}, {50}, {3}, {45}}, "Ac/L3/Current"},
    {{{}, {}, {}, {50}, {3}, {36}}, "Ac/L3/Voltage"},
    {{{}, {}, {}, {50}, {3}, {18}}, "Ac/L3/Power"},
};

bool DZGridMeter::handles(ValueID zwaveValueId)
{
    for (const auto& v : DZGridMeter::valueMapping)
    {
        if (DZUtil::match(zwaveValueId, v.first)) {
            return true;
        }
    }
    return false;
}

DZGridMeter::DZGridMeter(ValueID zwaveValueId) : DZValue(zwaveValueId)
{
    for (const auto& v : DZGridMeter::valueMapping)
    {
        if (DZUtil::match(zwaveValueId, v.first))
        {
            this->path = v.second;
            break;
        }
    }
}

void DZGridMeter::publish()
{
    this->addConstAux("ProductName", Manager::Get()->GetNodeProductName(this->zwaveValueId.GetHomeId(), this->zwaveValueId.GetNodeId()));
    if (DZItem::get(this->getServiceName(), "CustomName") == NULL)
    {
        this->addAuxiliary(new DZNodeName(this->zwaveValueId.GetHomeId(), this->zwaveValueId.GetNodeId(), this->getServiceName(), "CustomName"));
    }
    this->addConstAux("Connected", true);
    this->addConstAux("DeviceInstance", this->zwaveValueId.GetNodeId());
    this->addConstAux("ProductId", VE_PROD_NOT_SET);
    this->addConstAux("Mgmt/ProcessName", pltProgramName());
    this->addConstAux("Mgmt/ProcessVersion", pltProgramVersion());
    this->addConstAux("Mgmt/Connection", pltGetSerialDevice());

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

template<typename T> void DZGridMeter::addConstAux(string path, T value)
{
    if (DZItem::get(this->getServiceName(), path) == NULL)
    {
        this->addAuxiliary(new DZConstValue(this->getServiceName(), path, value));
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
        if(current != NULL)
        {
            totalCurrent += current->veItem->variant.value.Float;
        }
    }

    DZConstValue* current = static_cast<DZConstValue*>(DZItem::get(this->getServiceName(), "Ac/Current"));
    if (current == NULL)
    {
        this->addAuxiliary(new DZConstValue(this->getServiceName(), "Ac/Current", totalCurrent, 0, "A"));
    }
    else
    {
        current->set(totalCurrent);
    }

    // Power
    float totalPower = 0;
    for (const string &path : {"Ac/L1/Power", "Ac/L2/Power", "Ac/L3/Power"})
    {
        DZGridMeter* power = static_cast<DZGridMeter*>(DZItem::get(this->getServiceName(), path));
        if(power != NULL)
        {
            totalPower += power->veItem->variant.value.Float;
        }
    }

    DZConstValue* power = static_cast<DZConstValue*>(DZItem::get(this->getServiceName(), "Ac/Power"));
    if (power == NULL)
    {
        this->addAuxiliary(new DZConstValue(this->getServiceName(), "Ac/Power", totalPower, 0, "W"));
    }
    else
    {
        power->set(totalPower);
    }
}
