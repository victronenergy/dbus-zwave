extern "C" {
#include <velib/types/variant.h>
#include <velib/types/variant_print.h>
#include <velib/types/ve_dbus_item.h>
#include <velib/types/ve_item_def.h>
#include <velib/types/ve_values.h>
#include <velib/utils/ve_item_utils.h>
}

#include "dz_item.h"
#include "dz_constvalue.h"

using std::string;

static VeVariantUnitFmt unit = {0, ""};

DZConstValue::DZConstValue(string serviceName, string path) : DZItem() {
    this->serviceName = serviceName;
    this->path = path;
}

DZConstValue::DZConstValue(string serviceName, string path, int value) : DZConstValue(serviceName, path) {
    veVariantSn32(&this->value, +value);
}

DZConstValue::DZConstValue(string serviceName, string path, bool value) : DZConstValue(serviceName, path) {
    veVariantBit(&this->value, 1, value);
}

DZConstValue::DZConstValue(string serviceName, string path, float value) : DZConstValue(serviceName, path) {
    veVariantFloat(&this->value, value);
}

DZConstValue::DZConstValue(string serviceName, string path, string value) : DZConstValue(serviceName, path) {
    veVariantHeapStr(&this->value, value.c_str());
}

void DZConstValue::publish()
{
    this->description = "Constant value";
    this->veFmt = &unit;

    DZItem::publish();

    veItemOwnerSet(this->veItem, &this->value);
}

string DZConstValue::getServiceName()
{
    return this->serviceName;
}

string DZConstValue::getPath()
{
    return this->path;
}

void DZConstValue::onNotification(const Notification* _notification) {}
