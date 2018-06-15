#ifndef DZ_CONFIGURATOR_H
#define DZ_CONFIGURATOR_H

#include <Notification.h>
#include <value_classes/ValueID.h>

using OpenZWave::Notification;
using OpenZWave::ValueID;

class DZConfigurator
{
  public:
    static void onZwaveNotification(const Notification* _notification, void* _context);

    explicit DZConfigurator(ValueID zwaveValueId);

    virtual void bind();

  protected:
    ValueID zwaveValueId;

    ~DZConfigurator();

    virtual void onZwaveNotification(const Notification* _notification);

    virtual void update() = 0;
};

#endif
