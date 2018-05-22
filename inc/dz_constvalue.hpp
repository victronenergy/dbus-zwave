#ifndef DZ_CONSTVALUE_H
#define DZ_CONSTVALUE_H

#include <string>

extern "C" {
#include <velib/types/variant.h>
}

#include <Defs.h>
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

    DZConstValue(string serviceName, string path, int value, uint8 decimals, string unit);
    DZConstValue(string serviceName, string path, unsigned int value, uint8 decimals, string unit);
    DZConstValue(string serviceName, string path, bool value, uint8 decimals, string unit);
    DZConstValue(string serviceName, string path, float value, uint8 decimals, string unit);
    DZConstValue(string serviceName, string path, const char* value, uint8 decimals, string unit);
    DZConstValue(string serviceName, string path, string value, uint8 decimals, string unit);

    virtual void publish() override;
    virtual string getServiceName() override;
    virtual string getPath() override;

    void set(int value);
    void set(unsigned int value);
    void set(bool value);
    void set(float value);
    void set(const char* value);
    void set(string value);
    void setDecimals(uint8 decimals);
    void setUnit(string unit);

  protected:
    string    serviceName;
    string    path;
    VeVariant value;

    DZConstValue(string serviceName, string path, uint8 decimals, string unit);
    DZConstValue(string serviceName, string path);
    ~DZConstValue();

    virtual void onZwaveNotification(const Notification* _notification) override;
    virtual void onVeItemChanged() override;

    void update();
};

#endif
