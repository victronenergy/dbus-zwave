#ifndef DZ_CONSTVALUE_H
#define DZ_CONSTVALUE_H

extern "C" {
#include <velib/types/variant.h>
}

#include <Notification.h>

#include "dz_item.hpp"

using std::string;

class DZConstValue : public DZItem
{
  public:
    DZConstValue(string serviceName, string path, int value);
    DZConstValue(string serviceName, string path, unsigned int value);
    DZConstValue(string serviceName, string path, bool value);
    DZConstValue(string serviceName, string path, float value);
    DZConstValue(string serviceName, string path, const char* value);
    DZConstValue(string serviceName, string path, string value);

    virtual void publish() override;
    virtual string getServiceName() override;
    virtual string getPath() override;

  protected:
    string    serviceName;
    string    path;
    VeVariant value;

    DZConstValue(string serviceName, string path);

    virtual void onZwaveNotification(const Notification* _notification) override;
    virtual void onVeItemChanged() override;
};

#endif
