#include <map>
#include <string>
#include <tuple>
#include <vector>

extern "C" {
#include <velib/utils/ve_logger.h>
}

#include <Defs.h>
#include <Manager.h>
#include <value_classes/ValueID.h>

#include "configurators/dz_aeotec_zw095.hpp"
#include "dz_configurator.hpp"

using OpenZWave::Manager;
using OpenZWave::ValueID;
using std::map;
using std::string;
using std::tuple;
using std::vector;

map<tuple<uint8, uint8, uint8>, int32> DZAeotecZw095::configMapping = {
    {std::make_tuple(112, 1, 101), 0b00000000000001110011111100000000}, // TODO: remove GNU-ism
    {std::make_tuple(112, 1, 111), 20},
    {std::make_tuple(112, 1, 3), 1},
};

bool DZAeotecZw095::handles(ValueID zwaveValueId)
{
    return
        Manager::Get()->GetNodeManufacturerId(zwaveValueId.GetHomeId(), zwaveValueId.GetNodeId()) == "0x0086" &&
        (
            Manager::Get()->GetNodeProductType(zwaveValueId.GetHomeId(), zwaveValueId.GetNodeId()) == "0x0002" ||
            Manager::Get()->GetNodeProductType(zwaveValueId.GetHomeId(), zwaveValueId.GetNodeId()) == "0x0102" ||
            Manager::Get()->GetNodeProductType(zwaveValueId.GetHomeId(), zwaveValueId.GetNodeId()) == "0x0202"
        ) &&
        Manager::Get()->GetNodeProductId(zwaveValueId.GetHomeId(), zwaveValueId.GetNodeId()) == "0x005f" &&
        DZAeotecZw095::configMapping.count(DZAeotecZw095::zwaveValueIdToTuple(zwaveValueId));
}

tuple<uint8, uint8, uint8> DZAeotecZw095::zwaveValueIdToTuple(ValueID zwaveValueId)
{
    return std::make_tuple(
        zwaveValueId.GetCommandClassId(),
        zwaveValueId.GetInstance(),
        zwaveValueId.GetIndex()
    );
}

DZAeotecZw095::DZAeotecZw095(ValueID zwaveValueId) : DZConfigurator(zwaveValueId)
{
    this->configValue = DZAeotecZw095::configMapping[DZAeotecZw095::zwaveValueIdToTuple(zwaveValueId)];
}

void DZAeotecZw095::update()
{
    switch ((this->zwaveValueId).GetType())
    {

        case ValueID::ValueType_Int:
        {
            int32 currentValue;
            Manager::Get()->GetValueAsInt(this->zwaveValueId, &currentValue);
            if (currentValue != this->configValue)
            {
                Manager::Get()->SetValue(this->zwaveValueId, this->configValue);
            }
            break;
        }

        case ValueID::ValueType_List:
        {
            int32 currentValue;
            Manager::Get()->GetValueListSelection(this->zwaveValueId, &currentValue);
            if (currentValue != this->configValue)
            {
                vector<int32> possibleIndexes;
                    Manager::Get()->GetValueListValues(zwaveValueId, &possibleIndexes);
                    vector<string> possibleValues;
                    Manager::Get()->GetValueListItems(zwaveValueId, &possibleValues);
                    vector<int32>::iterator itIndex = possibleIndexes.begin();
                    vector<string>::iterator itValue = possibleValues.begin();
                    while (itIndex != possibleIndexes.end() && itValue != possibleValues.end())
                    {
                        if (*itIndex == this->configValue)
                        {
                            Manager::Get()->SetValueListSelection(this->zwaveValueId, *itValue);
                            break;
                        }
                        ++itIndex;
                        ++itValue;
                    }
            }
            break;
        }

        default:
        {
            break;
        }
    }
}
