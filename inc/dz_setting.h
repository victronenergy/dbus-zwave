#ifndef _DZ_SETTING_H
#define _DZ_SETTING_H

#include <string>

extern "C" {
#include <velib/types/variant.h>
}

#include <Notification.h>

#include "dz_item.h"

using std::string;

class DZSetting : protected DZItem
{
  public:
    static void onValueChanged(VeItem* veItem);

    DZSetting(string settingPath, int value);
    DZSetting(string settingPath, bool value);
    DZSetting(string settingPath, float value);
    DZSetting(string settingPath, string value);

    virtual void publish() override;
    virtual string getServiceName() override;
    virtual string getPath() override;

  protected:
    string    settingPath;
    VeVariant value;

    DZSetting(string settingPath);

    virtual void onZwaveNotification(const Notification* _notification) override;
    virtual void onVeItemChanged() override;
};

#endif
