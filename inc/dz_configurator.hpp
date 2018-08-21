#ifndef DZ_CONFIGURATOR_H
#define DZ_CONFIGURATOR_H

#include <chrono>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <tuple>

#include <Notification.h>
#include <value_classes/ValueID.h>

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
    struct MatchSpec {
      set<string> manufacturerIds;
      set<string> productTypes;
      set<string> productIds;
      set<uint8> commandClassIds;
      set<uint8> instances;
      set<uint8> indexes;
    };
    typedef map<MatchSpec, int32> ConfigValues; // TODO: support all valid Zwave types instead of just int32
    typedef map<MatchSpec, uint8> PollIntensities;

    static const seconds defaultCooldownTime;

    static bool match(ValueID zwaveValueId, ConfigValues configMap, PollIntensities pollingMap);
    static bool match(ValueID zwaveValueId, MatchSpec spec);

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
    int32                    configValue;
    bool                     hasPollingIntensity;
    uint8                    pollingIntensity;
};

namespace std
{
  template<> struct less<DZConfigurator::MatchSpec>
  {
    bool operator() (const DZConfigurator::MatchSpec& lhs, const DZConfigurator::MatchSpec& rhs) const
    {
      return
        tie(lhs.manufacturerIds, lhs.productTypes, lhs.productIds, lhs.commandClassIds, lhs.instances, lhs.indexes)
        <
        tie(rhs.manufacturerIds, rhs.productTypes, rhs.productIds, rhs.commandClassIds, rhs.instances, rhs.indexes);
    }
  };
}

#endif
