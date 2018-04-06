#ifndef _DZ_CONSTVALUE_H
#define _DZ_CONSTVALUE_H

#include "dz_item.h"

using std::string;

class DZConstValue : protected DZItem
{
  public:
    DZConstValue(string serviceName, string path, int value);
    DZConstValue(string serviceName, string path, bool value);
    DZConstValue(string serviceName, string path, float value);
    DZConstValue(string serviceName, string path, string value);

    void publish() override;
    string getServiceName() override;
    string getPath() override;

  protected:
    string    serviceName;
    string    path;
    VeVariant value;

    DZConstValue(string serviceName, string path);

    void onNotification(const Notification* _notification) override;
};

#endif
