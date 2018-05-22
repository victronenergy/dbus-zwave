#include <string>

extern "C" {
#include <velib/types/variant.h>
#include <velib/types/variant_print.h>
#include <velib/types/ve_dbus_item.h>
#include <velib/types/ve_item_def.h>
#include <velib/types/ve_values.h>
#include <velib/utils/ve_item_utils.h>
}

#include "dz_item.hpp"
#include "dz_constvalue.hpp"

using std::string;

static VeVariantUnitFmt unit = {0, ""};

DZConstValue::DZConstValue(string serviceName, string path) : DZItem() {
    this->serviceName = serviceName;
    this->path = path;
}

DZConstValue::DZConstValue(string serviceName, string path, int value) : DZConstValue(serviceName, path) {
    this->set(value);
    this->update();
}

DZConstValue::DZConstValue(string serviceName, string path, unsigned int value) : DZConstValue(serviceName, path) {
    this->set(value);
    this->update();
}

DZConstValue::DZConstValue(string serviceName, string path, bool value) : DZConstValue(serviceName, path) {
    this->set(value);
    this->update();
}

DZConstValue::DZConstValue(string serviceName, string path, float value) : DZConstValue(serviceName, path) {
    this->set(value);
    this->update();
}

DZConstValue::DZConstValue(string serviceName, string path, const char* value) : DZConstValue(serviceName, path) {
    this->set(value);
    this->update();
}

DZConstValue::DZConstValue(string serviceName, string path, string value) : DZConstValue(serviceName, path) {
    this->set(value);
    this->update();
}

void DZConstValue::publish()
{
    this->description = "Constant value";
    this->veFmt = &unit;

    DZItem::publish();
    this->update();
}

string DZConstValue::getServiceName()
{
    return this->serviceName;
}

string DZConstValue::getPath()
{
    return this->path;
}

void DZConstValue::set(int value) {
    veVariantSn32(&(this->value), +value);
}

void DZConstValue::set(unsigned int value) {
    veVariantUn32(&(this->value), +value);
}

void DZConstValue::set(bool value){
    veVariantUn8(&(this->value), value);
}

void DZConstValue::set(float value) {
    veVariantFloat(&(this->value), value);
}

void DZConstValue::set(const char* value) {
    veVariantStr(&(this->value), value);
}

void DZConstValue::set(string value) {
    veVariantHeapStr(&(this->value), value.c_str());
}

void DZConstValue::update() {
    if (this->isPublished())
    {
        veItemOwnerSet(this->veItem, &(this->value));
    }
}

void DZConstValue::onZwaveNotification(const Notification* _notification) {}
void DZConstValue::onVeItemChanged() {}
