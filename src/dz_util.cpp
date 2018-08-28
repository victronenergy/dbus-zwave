#include <set>
#include <string>
#include <vector>

extern "C" {
#include <velib/types/variant.h>
}

#include <Manager.h>
#include <value_classes/ValueID.h>

#include "dz_util.hpp"

using OpenZWave::Manager;
using OpenZWave::ValueID;
using std::set;
using std::string;
using std::ostringstream;
using std::vector;

bool DZUtil::match(ValueID valueId, MatchSpec spec)
{
    Manager* m = Manager::Get();
    uint32 hid = valueId.GetHomeId();
    uint8 nid = valueId.GetNodeId();
    return
        (!spec.manufacturerIds.size() || spec.manufacturerIds.count(m->GetNodeManufacturerId(hid, nid)))
        &&
        (!spec.productTypes.size() || spec.productTypes.count(m->GetNodeProductType(hid, nid)))
        &&
        (!spec.productIds.size() || spec.productIds.count(m->GetNodeProductId(hid, nid)))
        &&
        (!spec.commandClassIds.size() || spec.commandClassIds.count(valueId.GetCommandClassId()))
        &&
        (!spec.instances.size() || spec.instances.count(valueId.GetInstance()))
        &&
        (!spec.indexes.size() || spec.indexes.count(valueId.GetIndex()));
}

string DZUtil::path(uint32 homeId)
{
    ostringstream path;
    path << "Interfaces/" << +homeId;
    return path.str();
}

string DZUtil::path(uint32 homeId, uint8 nodeId)
{
    ostringstream path;
    path << DZUtil::path(homeId) << "/Devices/" << +nodeId;
    return path.str();
}

string DZUtil::path(uint32 homeId, uint8 nodeId, uint8 commandClassId)
{
    ostringstream path;
    path << DZUtil::path(homeId, nodeId) << "/CommandClasses/" << +commandClassId;
    return path.str();
}

string DZUtil::path(uint32 homeId, uint8 nodeId, uint8 commandClassId, uint8 instance)
{
    ostringstream path;
    path << DZUtil::path(homeId, nodeId, commandClassId) << "/Instances/" << +instance;
    return path.str();
}

string DZUtil::path(uint32 homeId, uint8 nodeId, uint8 commandClassId, uint8 instance, uint8 index)
{
    ostringstream path;
    path << DZUtil::path(homeId, nodeId, commandClassId, instance) << "/Indexes/" << +index;
    return path.str();
}

string DZUtil::path(ValueID valueId)
{
    return DZUtil::path(
        valueId.GetHomeId(),
        valueId.GetNodeId(),
        valueId.GetCommandClassId(),
        valueId.GetInstance(),
        valueId.GetIndex()
    );
}

bool DZUtil::setZwaveValueByVariant(ValueID zwaveValueId, VeVariant veVariant)
{
    switch((zwaveValueId).GetType())
    {
        case ValueID::ValueType_Bool:
        {
            if (veVariant.type.tp != VE_UN8)
            {
                logE("DZUtil", "Invalid variant type for ValueType_Bool. Should be %d, got %d", VE_UN8, veVariant.type.tp);
                return false;
            }
            bool currentValue;
            Manager::Get()->GetValueAsBool(zwaveValueId, &currentValue);
            bool newValue = veVariant.value.UN8;
            return newValue == currentValue || Manager::Get()->SetValue(zwaveValueId, newValue);
        }

        case ValueID::ValueType_Byte:
        {
            if (veVariant.type.tp != VE_UN8)
            {
                logE("DZUtil", "Invalid variant type for ValueType_Byte. Should be %d, got %d", VE_UN8, veVariant.type.tp);
                return false;
            }
            uint8 currentValue;
            Manager::Get()->GetValueAsByte(zwaveValueId, &currentValue);
            uint8 newValue = veVariant.value.UN8;
            return newValue == currentValue || Manager::Get()->SetValue(zwaveValueId, newValue);
        }

        case ValueID::ValueType_Decimal:
        {
            if (veVariant.type.tp != VE_FLOAT)
            {
                logE("DZUtil", "Invalid variant type for ValueType_Decimal. Should be %d, got %d", VE_FLOAT, veVariant.type.tp);
                return false;
            }
            string currentValue;
            Manager::Get()->GetValueAsString(zwaveValueId, &currentValue);
            float newValue = veVariant.value.Float;
            return newValue == std::stod(currentValue.c_str()) || Manager::Get()->SetValue(zwaveValueId, newValue);
        }

        case ValueID::ValueType_Int:
        {
            if (veVariant.type.tp != VE_SN32)
            {
                logE("DZUtil", "Invalid variant type for ValueType_Int. Should be %d, got %d", VE_SN32, veVariant.type.tp);
                return false;
            }
            int32 currentValue;
            Manager::Get()->GetValueAsInt(zwaveValueId, &currentValue);
            int32 newValue = veVariant.value.SN32;
            return newValue == currentValue || Manager::Get()->SetValue(zwaveValueId, newValue);
        }

        case ValueID::ValueType_List:
        {
            if (veVariant.type.tp == VE_HEAP_STR)
            {
                string currentValue;
                Manager::Get()->GetValueListSelection(zwaveValueId, &currentValue);
                string newValue = string((char*) veVariant.value.CPtr);
                return newValue == currentValue || Manager::Get()->SetValue(zwaveValueId, newValue);
            }
            else if (veVariant.type.tp == VE_SN32)
            {
                int32 currentIndex;
                Manager::Get()->GetValueListSelection(zwaveValueId, &currentIndex);
                int32 newIndex = veVariant.value.SN32;
                if (newIndex == currentIndex)
                {
                    return true;
                }

                vector<int32> possibleIndexes;
                Manager::Get()->GetValueListValues(zwaveValueId, &possibleIndexes);
                vector<string> possibleValues;
                Manager::Get()->GetValueListItems(zwaveValueId, &possibleValues);
                vector<int32>::iterator itIndex = possibleIndexes.begin();
                vector<string>::iterator itValue = possibleValues.begin();
                while (itIndex != possibleIndexes.end() && itValue != possibleValues.end())
                {
                    if (*itIndex == newIndex)
                    {
                        return Manager::Get()->SetValueListSelection(zwaveValueId, *itValue);
                    }
                    ++itIndex;
                    ++itValue;
                }
                return false;
            }
            else
            {
                logE("DZUtil", "Invalid variant type for ValueType_List. Should be %d or %d, got %d", VE_HEAP_STR, VE_SN32, veVariant.type.tp);
                return false;
            }
        }

        case ValueID::ValueType_Schedule:
        {
            logE("DZUtil", "Tried to set unsupported ValueType_Shedule");
            return false;
        }

        case ValueID::ValueType_Short:
        {
            if (veVariant.type.tp != VE_SN16)
            {
                logE("DZUtil", "Invalid variant type for ValueType_Short. Should be %d, got %d", VE_SN16, veVariant.type.tp);
                return false;
            }
            int16 currentValue;
            Manager::Get()->GetValueAsShort(zwaveValueId, &currentValue);
            int16 newValue = veVariant.value.SN16;
            return newValue == currentValue || Manager::Get()->SetValue(zwaveValueId, newValue);
        }

        case ValueID::ValueType_String:
        {
            if (veVariant.type.tp != VE_HEAP_STR)
            {
                logE("DZUtil", "Invalid variant type for ValueType_String. Should be %d, got %d", VE_HEAP_STR, veVariant.type.tp);
                return false;
            }
            string currentValue;
            Manager::Get()->GetValueAsString(zwaveValueId, &currentValue);
            string newValue = string((char*) veVariant.value.CPtr);
            return newValue == currentValue || Manager::Get()->SetValue(zwaveValueId, newValue);
        }

        case ValueID::ValueType_Button:
        {
            if (veVariant.type.tp != VE_UN8)
            {
                logE("DZUtil", "Invalid variant type for ValueType_Button. Should be %d, got %d", VE_UN8, veVariant.type.tp);
                return false;
            }
            bool currentValue;
            Manager::Get()->GetValueAsBool(zwaveValueId, &currentValue);
            bool newValue = veVariant.value.UN8;
            if (newValue == currentValue)
            {
                return true;
            }
            if (veVariant.value.UN8)
            {
                return Manager::Get()->PressButton(zwaveValueId);
            }
            else
            {
                return Manager::Get()->ReleaseButton(zwaveValueId);
            }
        }

        case ValueID::ValueType_Raw:
        {
            if (veVariant.type.tp != VE_BUF)
            {
                logE("DZUtil", "Invalid variant type for ValueType_Raw. Should be %d, got %d", VE_BUF, veVariant.type.tp);
                return false;
            }
            uint8* currentValue;
            uint8 currentLength;
            Manager::Get()->GetValueAsRaw(zwaveValueId, &currentValue, &currentLength);
            uint8* newValue = (uint8*) veVariant.value.CPtr;
            uint8 newLength = veVariant.type.len;
            if (newLength == currentLength || memcmp(newValue, currentValue, newLength) == 0)
            {
                return true;
            }
            return Manager::Get()->SetValue(zwaveValueId, newValue, newLength);
        }

        default:
        {
            return false;
        }
    }
}
