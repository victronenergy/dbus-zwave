#ifndef DZ_BUTTON_H
#define DZ_BUTTON_H

#include <functional>
#include <string>

#include <Notification.h>

#include "dz_item.hpp"

using std::function;
using std::string;

class DZButton : public DZItem
{
  public:
    explicit DZButton(string serviceName, string path, function<void(void*)> callback, void* context);
    explicit DZButton(string serviceName, string path, function<void()> callback);

    virtual void publish() override;
    virtual string getServiceName() override;
    virtual string getPath() override;

  protected:
    string                serviceName;
    string                path;
    function<void(void*)> callback;
    void*                 context;

    explicit DZButton(string serviceName, string path);

    virtual void onZwaveNotification(const Notification* _notification) override;
    virtual void onVeItemChanged() override;
};

#endif
