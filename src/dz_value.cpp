#include <map>
#include <string>

extern "C" {
#include <velib/types/variant.h>
#include <velib/types/variant_print.h>
#include <velib/types/ve_item_def.h>
#include <velib/types/ve_values.h>
#include <velib/utils/ve_item_utils.h>
#include <velib/utils/ve_logger.h>
}

#include <Defs.h>
#include <Manager.h>
#include <Notification.h>
#include <value_classes/ValueID.h>

#include "dz_value.hpp"
#include "dz_item.hpp"
#include "dz_util.hpp"

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
    this->veFmt->decimals = 0;
    string unit = Manager::Get()->GetValueUnits(this->zwaveValueId);
    if (unit.size() > 0)
    {
        unit = " " + unit;
    }
    this->veFmt->unit = strdup(unit.c_str());

    DZItem::publish();

    VeVariant veVariant;
    veItemSetMin(this->veItem, veVariantSn32(&veVariant, Manager::Get()->GetValueMin(this->zwaveValueId)));
    veItemSetMax(this->veItem, veVariantSn32(&veVariant, Manager::Get()->GetValueMax(this->zwaveValueId)));

    // TODO: Publish long description string somewhere on D-Bus?
    string valueHelp = Manager::Get()->GetValueHelp(this->zwaveValueId);
    if (valueHelp.size() > 0)
    {
        logI("DZValue", "%s", valueHelp.c_str());
    }

    this->update();
}

DZValue::~DZValue()
{
    if (this->isPublished())
    {
        free(this->veFmt->unit);
        delete this->veFmt;
    }
}

void DZValue::onZwaveNotification(const Notification* _notification)
{
    if(_notification->GetValueID() == this->zwaveValueId)
    {
        switch (_notification->GetType())
        {
            case Notification::Type_ValueChanged:
            {
                this->update();
                break;
            }

            case Notification::Type_ValueRemoved:
            {
                delete this;
                break;
            }

            default:
            {
                break;
            }
        }
    }
}

void DZValue::onVeItemChanged()
{
    if (!DZUtil::setZwaveValueByVariant(this->zwaveValueId, this->veItem->variant))
    {
        this->update();
    }
}

string DZValue::getPath()
{
    return DZUtil::path(this->zwaveValueId);
}

void DZValue::update()
{
    VeVariant veVariant;
    switch ((this->zwaveValueId).GetType())
    {
        case ValueID::ValueType_Bool:
        {
            bool value;
            Manager::Get()->GetValueAsBool(this->zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantUn8(&veVariant, value));
            break;
        }

        case ValueID::ValueType_Byte:
        {
            uint8 value;
            Manager::Get()->GetValueAsByte(this->zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantUn8(&veVariant, +value));
            break;
        }

        case ValueID::ValueType_Decimal:
        {
            Manager::Get()->GetValueFloatPrecision(this->zwaveValueId, &this->veFmt->decimals);
            string value;
            Manager::Get()->GetValueAsString(this->zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantFloat(&veVariant, std::stod(value.c_str())));
            break;
        }

        case ValueID::ValueType_Int:
        {
            int32 value;
            Manager::Get()->GetValueAsInt(this->zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantSn32(&veVariant, +value));
            break;
        }

        case ValueID::ValueType_List:
        {
            int32 valueIndex;
            Manager::Get()->GetValueListSelection(this->zwaveValueId, &valueIndex);
            string valueName;
            Manager::Get()->GetValueListSelection(this->zwaveValueId, &valueName);
            char* oldUnit = this->veFmt->unit;
            this->veFmt->unit = strdup((" = " + valueName).c_str());
            veItemOwnerSet(this->veItem, veVariantSn32(&veVariant, +valueIndex));
            free(oldUnit);
            break;
        }

        case ValueID::ValueType_Schedule:
        {
            veItemOwnerSet(this->veItem, veVariantInvalidType(&veVariant, VE_UNKNOWN));
            break;
        }

        case ValueID::ValueType_Short:
        {
            int16 value;
            Manager::Get()->GetValueAsShort(this->zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantSn16(&veVariant, +value));
            break;
        }

        case ValueID::ValueType_String:
        {
            string value;
            Manager::Get()->GetValueAsString(this->zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantHeapStr(&veVariant, value.c_str()));
            break;
        }

        case ValueID::ValueType_Button:
        {
            bool value;
            Manager::Get()->GetValueAsBool(this->zwaveValueId, &value);
            veItemOwnerSet(this->veItem, veVariantUn8(&veVariant, value));
            break;
        }

        case ValueID::ValueType_Raw:
        {
            uint8* value;
            uint8 length;
            Manager::Get()->GetValueAsRaw(this->zwaveValueId, &value, &length);
            veItemOwnerSet(this->veItem, veVariantBuf(&veVariant, value, length));
            break;
        }
    }
}
