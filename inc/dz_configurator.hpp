#ifndef DZ_CONFIGURATOR_H
#define DZ_CONFIGURATOR_H

#include <chrono>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <tuple>

extern "C" {
#include <velib/types/variant.h>
}

#include <Notification.h>
#include <value_classes/ValueID.h>

#include "dz_util.hpp"

using OpenZWave::Notification;
using OpenZWave::ValueID;
using std::chrono::seconds;
using std::chrono::system_clock;
using std::map;
using std::set;
using std::string;

class DZConfigurator
{
  public:
    typedef map<DZUtil::MatchSpec, DZUtil::Variant> ConfigValues;
    typedef map<DZUtil::MatchSpec, uint8> PollIntensities;

    static const seconds defaultCooldownTime;

    static bool match(ValueID zwaveValueId, ConfigValues configMap, PollIntensities pollingMap);

    static void onZwaveNotification(const Notification* _notification, void* _context);

    explicit DZConfigurator(ValueID zwaveValueId);
    explicit DZConfigurator(ValueID zwaveValueId, seconds cooldownTime);
    virtual ~DZConfigurator();

    virtual void bind() { return bind(false); }
    virtual void bind(bool awaitQueryComplete);

  protected:
    ValueID zwaveValueId;
    seconds cooldownTime;

    virtual ConfigValues    getConfigMap()  = 0;
    virtual PollIntensities getPollingMap() = 0;

    virtual void onZwaveNotification(const Notification* _notification);
    virtual void update();

  private:
    pthread_mutex_t          criticalSection;
    bool                     initCompleted;
    system_clock::time_point lastUpdate;
    bool                     hasConfigValue;
    DZUtil::Variant          configValue;
    bool                     hasPollingIntensity;
    uint8                    pollingIntensity;
};

#endif
