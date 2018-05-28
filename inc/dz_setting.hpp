#ifndef DZ_SETTING_H
#define DZ_SETTING_H

#include <string>

extern "C" {
#include <velib/types/variant.h>
}

#include <Notification.h>

#include "dz_item.hpp"

using std::string;

class DZSetting : public DZItem
{
  public:
    static void onValueChanged(VeItem* veItem);

    explicit DZSetting(string settingPath, int value);
    explicit DZSetting(string settingPath, bool value);
    explicit DZSetting(string settingPath, float value);
    explicit DZSetting(string settingPath, string value);

    virtual void publish() override;
    virtual string getServiceName() override;
    virtual string getPath() override;

  protected:
    string    settingPath;
    VeVariant value;

    explicit DZSetting(string settingPath);

    virtual void onZwaveNotification(const Notification* _notification) override;
    virtual void onVeItemChanged() override;
};

#endif
