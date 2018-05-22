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

#include "dz_item.hpp"
#include "dz_constvalue.hpp"

using std::string;

DZConstValue::DZConstValue(string serviceName, string path, int value) : DZConstValue(serviceName, path) {
    this->set(value);
}

DZConstValue::DZConstValue(string serviceName, string path, unsigned int value) : DZConstValue(serviceName, path) {
    this->set(value);
}

DZConstValue::DZConstValue(string serviceName, string path, bool value) : DZConstValue(serviceName, path) {
    this->set(value);
}

DZConstValue::DZConstValue(string serviceName, string path, float value) : DZConstValue(serviceName, path) {
    this->set(value);
}

DZConstValue::DZConstValue(string serviceName, string path, const char* value) : DZConstValue(serviceName, path) {
    this->set(value);
}

DZConstValue::DZConstValue(string serviceName, string path, string value) : DZConstValue(serviceName, path) {
    this->set(value);
}

DZConstValue::DZConstValue(string serviceName, string path, int value, uint8 decimals, string unit) : DZConstValue(serviceName, path, decimals, unit) {
    this->set(value);
}

DZConstValue::DZConstValue(string serviceName, string path, unsigned int value, uint8 decimals, string unit) : DZConstValue(serviceName, path, decimals, unit) {
    this->set(value);
}

DZConstValue::DZConstValue(string serviceName, string path, bool value, uint8 decimals, string unit) : DZConstValue(serviceName, path, decimals, unit) {
    this->set(value);
}

DZConstValue::DZConstValue(string serviceName, string path, float value, uint8 decimals, string unit) : DZConstValue(serviceName, path, decimals, unit) {
    this->set(value);
}

DZConstValue::DZConstValue(string serviceName, string path, const char* value, uint8 decimals, string unit) : DZConstValue(serviceName, path, decimals, unit) {
    this->set(value);
}

DZConstValue::DZConstValue(string serviceName, string path, string value, uint8 decimals, string unit) : DZConstValue(serviceName, path, decimals, unit) {
    this->set(value);
}

DZConstValue::DZConstValue(string serviceName, string path, uint8 decimals, string unit) : DZConstValue(serviceName, path) {
    this->setDecimals(decimals);
    this->setUnit(unit);
}

DZConstValue::DZConstValue(string serviceName, string path) : DZItem() {
    this->serviceName = serviceName;
    this->path = path;
    this->veFmt = new VeVariantUnitFmt();
    this->veFmt->decimals = 0;
    this->veFmt->unit = strdup("");
}

DZConstValue::~DZConstValue()
{
    free(this->veFmt->unit);
    delete this->veFmt;
}

void DZConstValue::publish()
{
    this->description = "Constant value";

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
    this->update();
}

void DZConstValue::set(unsigned int value) {
    veVariantUn32(&(this->value), +value);
    this->update();
}

void DZConstValue::set(bool value){
    veVariantUn8(&(this->value), value);
    this->update();
}

void DZConstValue::set(float value) {
    veVariantFloat(&(this->value), value);
    this->update();
}

void DZConstValue::set(const char* value) {
    veVariantStr(&(this->value), value);
    this->update();
}

void DZConstValue::set(string value) {
    veVariantHeapStr(&(this->value), value.c_str());
    this->update();
}

void DZConstValue::setDecimals(uint8 decimals)
{
    this->veFmt->decimals = decimals;
}
void DZConstValue::setUnit(string unit)
{
    if (unit.size() > 0)
    {
        unit = " " + unit;
    }
    char* previousUnit = this->veFmt->unit;
    this->veFmt->unit = strdup(unit.c_str());
    free(previousUnit);
}

void DZConstValue::update() {
    if (this->isPublished())
    {
        veItemOwnerSet(this->veItem, &(this->value));
    }
}

void DZConstValue::onZwaveNotification(const Notification* _notification) {}
void DZConstValue::onVeItemChanged() {}
