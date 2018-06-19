#include <functional>
#include <string>

extern "C" {
#include <velib/types/variant.h>
#include <velib/types/ve_item.h>
#include <velib/types/ve_item_def.h>
#include <velib/utils/ve_item_utils.h>
}

#include <Notification.h>

#include "dz_button.hpp"
#include "dz_item.hpp"

using std::function;
using std::string;

DZButton::DZButton(string serviceName, string path, function<void(void*)> callback, void* context) : DZButton(serviceName, path)
{
    this->callback = callback;
    this->context = context;
}

DZButton::DZButton(string serviceName, string path, function<void()> callback) : DZButton(serviceName, path)
{
    this->callback = [callback](void* context) {
        VE_UNUSED(context);
        callback();
    };
    this->context = NULL;
}

DZButton::DZButton(string serviceName, string path) : DZItem()
{
    this->serviceName = serviceName;
    this->path = path;
}

void DZButton::publish()
{
    this->description = "";
    this->veFmt = &veUnitNone;

    DZItem::publish();

    VeVariant veVariant;
    veItemOwnerSet(this->veItem, veVariantUn8(&veVariant, 0));
}

string DZButton::getServiceName()
{
    return this->serviceName;
}

string DZButton::getPath()
{
    return this->path;
}

void DZButton::onZwaveNotification(const Notification* _notification)
{
    VE_UNUSED(_notification);
}

void DZButton::onVeItemChanged()
{
    VeVariant veVariant;
    veVariantUn8(&veVariant, 0);
    if (!veVariantIsEqual(&this->veItem->variant, &veVariant))
    {
        this->callback(this->context);
        veItemOwnerSet(this->veItem, &veVariant);
    }
}
