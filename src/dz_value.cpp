extern "C" {
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
#include <velib/types/ve_values.h>
#include <velib/utils/ve_item_utils.h>
}

#include <Defs.h>
#include <Manager.h>
#include <Notification.h>
#include <value_classes/ValueID.h>

#include "dz_item.h"
#include "dz_value.h"

using OpenZWave::Manager;
using OpenZWave::Notification;
using OpenZWave::ValueID;
using std::map;
using std::string;

DZValue::DZValue(ValueID zwaveValueId) : zwaveValueId(zwaveValueId)
{
    this->zwaveValueId = zwaveValueId;
}

void DZValue::publish()
{
    this->description = Manager::Get()->GetValueLabel(this->zwaveValueId);

    // Set up formatting for value
    this->veFmt = new VeVariantUnitFmt();
    Manager::Get()->GetValueFloatPrecision(this->zwaveValueId, &(this->veFmt->decimals));
    string unit = Manager::Get()->GetValueUnits(this->zwaveValueId);
    this->veFmt->unit = new char[unit.length() + 1];
    strcpy(this->veFmt->unit, unit.c_str());

    DZItem::publish();

    // TODO implement min and max?
    //VeVariant veVariant;
    //veItemSetMin(publishedValue->veItem, veVariantSn32(&veVariant, Manager::Get()->GetValueMin(this->zwaveValueId)));
    //veItemSetMax(publishedValue->veItem, veVariantSn32(&veVariant, Manager::Get()->GetValueMax(this->zwaveValueId)));

    // TODO implement long description string?
    // Manager::Get()->GetValueHelp(this->zwaveValueId)

    this->update(this->zwaveValueId);
}

DZValue::~DZValue()
{
    delete [] this->veFmt->unit;
    delete this->veFmt;
}

void DZValue::onNotification(const Notification* _notification)
{
    if(_notification->GetValueID() == this->zwaveValueId)
    {
        switch (_notification->GetType())
        {
            case Notification::Type_ValueChanged:
            {
                this->update(_notification->GetValueID());
                break;
            }

            case Notification::Type_ValueRemoved:
            {
                delete this;
                break;
            }

            default:
            {
            }
        }
    }
}

string DZValue::getPath()
{
    return DZItem::path(this->zwaveValueId);
}

void DZValue::update(ValueID zwaveValueId) {
    VeVariant veVariant;
    switch ((zwaveValueId).GetType()) {
        case ValueID::ValueType_Bool:
        {
            bool value;
            Manager::Get()->GetValueAsBool(zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantBit(&veVariant, 1, value));
            break;
        }

        case ValueID::ValueType_Decimal:
        {
            string value;
            Manager::Get()->GetValueAsString(zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantFloat(&veVariant, std::stod(value.c_str())));
            break;
        }

        case ValueID::ValueType_String:
        {
            string value;
            Manager::Get()->GetValueAsString(zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantHeapStr(&veVariant, value.c_str()));
            break;
        }

        case ValueID::ValueType_Byte:
        {
            uint8 value;
            Manager::Get()->GetValueAsByte(zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantUn8(&veVariant, +value));
            break;
        }

        case ValueID::ValueType_Short:
        {
            int16 value;
            Manager::Get()->GetValueAsShort(zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantSn16(&veVariant, +value));
            break;
        }

        case ValueID::ValueType_Int:
        {
            int32 value;
            Manager::Get()->GetValueAsInt(zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantSn32(&veVariant, +value));
            break;
        }

        default:
        {
            // TODO add list type using veVariantFloatArray ?

            // 0 = ValueType_Bool        Boolean, true or false
            // 1 = ValueType_Byte        8-bit unsigned value
            // 2 = ValueType_Decimal     Represents a non-integer value as a string, to avoid floating point accuracy issues.
            // 3 = ValueType_Int         32-bit signed value
            // 4 = ValueType_List        List from which one value can be selected
            // 5 = ValueType_Schedule    Complex type used with the Climate Control Schedule command class
            // 6 = ValueType_Short       16-bit signed value
            // 7 = ValueType_String      Text string
            // 8 = ValueType_Button      A write-only value that is the equivalent of pressing a button to send a command to a device
            // 9 = ValueType_Raw         A collection of bytes
        }
    }
}
