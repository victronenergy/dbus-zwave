#include <string>
#include <utility>

extern "C" {
#include <velib/platform/plt.h>
#include <velib/types/variant.h>
#include <velib/types/variant_print.h>
#include <velib/types/ve_dbus_item.h>
#include <velib/types/ve_item_def.h>
#include <velib/types/ve_values.h>
#include <velib/utils/ve_logger.h>
#include <velib/utils/ve_item_utils.h>
}

#include "dz_setting.hpp"
#include "dz_item.hpp"

using std::string;
using std::ostringstream;

static VeVariantUnitFmt unit = {0, ""};

void DZSetting::onValueChanged(VeItem* veItem)
{
    logI("DZSetting", "setting changed!");
}

DZSetting::DZSetting(string settingPath) : DZItem() {
    this->settingPath = settingPath;
}

DZSetting::DZSetting(string settingPath, int value) : DZSetting(settingPath) {
    veVariantSn32(&this->value, +value);
}

DZSetting::DZSetting(string settingPath, bool value) : DZSetting(settingPath) {
    veVariantBit(&this->value, 1, value);
}

DZSetting::DZSetting(string settingPath, float value) : DZSetting(settingPath) {
    veVariantFloat(&this->value, value);
}

DZSetting::DZSetting(string settingPath, string value) : DZSetting(settingPath) {
    veVariantHeapStr(&this->value, value.c_str());
}

void DZSetting::publish()
{
    this->description = "Setting";
    this->veFmt = &unit;

    VeItem* veRoot = veItemGetOrCreateUid(veValueTree(), this->getServiceName().c_str());
    VeDbus* dbusConnection = veDbusConnectString(veDbusGetDefaultConnectString());

    // DBus failures are fatal
    if (!dbusConnection)
    {
        logE("DZSetting", "dbus connection failed");
        pltExit(5);
    }

    veDbusSetListeningDbus(dbusConnection);

    if (!veDbusAddRemoteService(this->getServiceName().c_str(), veRoot, true))
    {
        logE("DZSetting", "veDbusAddRemoteService failed");
        pltExit(1);
    }

    DZItem::setService(std::make_pair(dbusConnection, veRoot));

    DZItem::publish();

    veItemSetChanged(this->veItem, &(DZSetting::onValueChanged));

    VeVariant veVariantZero;
    veVariantSn32(&veVariantZero, 0);
    if (!veDBusAddLocalSetting(this->veItem, &this->value, &veVariantZero, &veVariantZero, false))
    {
        logE("DZSetting", "veDBusAddLocalSetting failed");
        pltExit(1);
    }
}

string DZSetting::getServiceName()
{
    return "com.victronenergy.settings";
}

string DZSetting::getPath()
{
    ostringstream path;
    path << "Settings/" << this->settingPath;
    return path.str();
}

void DZSetting::onZwaveNotification(const Notification* _notification) {}
void DZSetting::onVeItemChanged() {}
