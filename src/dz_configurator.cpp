#include <chrono>
#include <string>
#include <vector>

extern "C" {
#include <velib/utils/ve_logger.h>
}

#include <Manager.h>
#include <Notification.h>
#include <value_classes/ValueID.h>

#include "dz_configurator.hpp"

using OpenZWave::Manager;
using OpenZWave::Notification;
using OpenZWave::ValueID;
using std::chrono::seconds;
using std::chrono::system_clock;
using std::vector;

const seconds DZConfigurator::defaultCooldownTime = seconds(30);

bool DZConfigurator::match(ValueID zwaveValueId, ConfigValues configMap, PollIntensities pollingMap)
{
    for (const auto& c : configMap)
    {
        if (DZConfigurator::match(zwaveValueId, c.first)) {
            return true;
        }
    }
    for (const auto& p : pollingMap)
    {
        if (DZConfigurator::match(zwaveValueId, p.first)) {
            return true;
        }
    }
    return false;
}

bool DZConfigurator::match(ValueID zwaveValueId, MatchSpec spec)
{
    Manager* m = Manager::Get();
    uint32 hid = zwaveValueId.GetHomeId();
    uint8 nid = zwaveValueId.GetNodeId();
    return
        (!spec.manufacturerIds.size() || spec.manufacturerIds.count(m->GetNodeManufacturerId(hid, nid)))
        &&
        (!spec.productTypes.size() || spec.productTypes.count(m->GetNodeProductType(hid, nid)))
        &&
        (!spec.productIds.size() || spec.productIds.count(m->GetNodeProductId(hid, nid)))
        &&
        (!spec.commandClassIds.size() || spec.commandClassIds.count(zwaveValueId.GetCommandClassId()))
        &&
        (!spec.instances.size() || spec.instances.count(zwaveValueId.GetInstance()))
        &&
        (!spec.indexes.size() || spec.indexes.count(zwaveValueId.GetIndex()));
}

void DZConfigurator::onZwaveNotification(const Notification* _notification, void* _context)
{
    return static_cast<DZConfigurator*>(_context)->onZwaveNotification(_notification);
}

DZConfigurator::DZConfigurator(ValueID zwaveValueId, seconds cooldownTime) : DZConfigurator(zwaveValueId)
{
    this->cooldownTime = cooldownTime;
}

DZConfigurator::DZConfigurator(ValueID zwaveValueId) : zwaveValueId(zwaveValueId)
{
    this->initCompleted = false;
    this->cooldownTime = DZConfigurator::defaultCooldownTime;
    this->zwaveValueId = zwaveValueId;
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&this->criticalSection, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
}

void DZConfigurator::bind(bool awaitQueryComplete)
{
    this->hasConfigValue = false;
    for (const auto& c : this->getConfigMap())
    {
        if (DZConfigurator::match(zwaveValueId, c.first))
        {
            this->configValue = c.second;
            this->hasConfigValue = true;
            break;
        }
    }
    this->hasPollingIntensity = false;
    for (const auto& p : this->getPollingMap())
    {
        if (DZConfigurator::match(zwaveValueId, p.first))
        {
            this->pollingIntensity = p.second;
            this->hasPollingIntensity = true;
            break;
        }
    }
    if (awaitQueryComplete)
    {
        Manager::Get()->AddWatcher(DZConfigurator::onZwaveNotification, (void*) this);
    }
    else
    {
        this->initCompleted = true;
        this->lastUpdate = system_clock::now();
        Manager::Get()->AddWatcher(DZConfigurator::onZwaveNotification, (void*) this);
        this->update();
        if (this->hasPollingIntensity)
        {
            Manager::Get()->EnablePoll(this->zwaveValueId, this->pollingIntensity);
        }
    }
}

DZConfigurator::~DZConfigurator()
{
    Manager::Get()->RemoveWatcher(DZConfigurator::onZwaveNotification, (void*) this);
}

void DZConfigurator::onZwaveNotification(const Notification* _notification)
{
    switch (_notification->GetType())
    {
        case Notification::Type_AwakeNodesQueried:
        case Notification::Type_AllNodesQueried:
        case Notification::Type_AllNodesQueriedSomeDead:
        {
            pthread_mutex_lock(&this->criticalSection);
            if (!this->initCompleted)
            {
                this->initCompleted = true;
                this->lastUpdate = system_clock::now();
                this->update();
                if (this->hasPollingIntensity)
                {
                    Manager::Get()->EnablePoll(this->zwaveValueId, this->pollingIntensity);
                }
            }
            pthread_mutex_unlock(&this->criticalSection);
            break;
        }

        default:
        {
            break;
        }
    }
    if (_notification->GetValueID() == this->zwaveValueId)
    {
        switch (_notification->GetType())
        {
            case Notification::Type_ValueChanged:
            {
                if (this->hasConfigValue)
                {
                    pthread_mutex_lock(&this->criticalSection);
                    if (this->initCompleted)
                    {
                        system_clock::time_point now = system_clock::now();
                        if (now - this->lastUpdate > this->cooldownTime)
                        {
                            this->lastUpdate = now;
                            this->update();
                        }
                    }
                    pthread_mutex_unlock(&this->criticalSection);
                }
                break;
            }

            case Notification::Type_ValueRemoved:
            {
                delete this;
                break;
            }

            default:
            {
                break;
            }
        }
    }
}

void DZConfigurator::update()
{
    if (!this->hasConfigValue)
    {
        return;
    }
    switch ((this->zwaveValueId).GetType())
    {
        case ValueID::ValueType_Int:
        {
            int32 currentValue;
            Manager::Get()->GetValueAsInt(this->zwaveValueId, &currentValue);
            if (currentValue != this->configValue)
            {
                Manager::Get()->SetValue(this->zwaveValueId, this->configValue);
            }
            break;
        }

        case ValueID::ValueType_List:
        {
            int32 currentValue;
            Manager::Get()->GetValueListSelection(this->zwaveValueId, &currentValue);
            if (currentValue != this->configValue)
            {
                vector<int32> possibleIndexes;
                    Manager::Get()->GetValueListValues(zwaveValueId, &possibleIndexes);
                    vector<string> possibleValues;
                    Manager::Get()->GetValueListItems(zwaveValueId, &possibleValues);
                    vector<int32>::iterator itIndex = possibleIndexes.begin();
                    vector<string>::iterator itValue = possibleValues.begin();
                    while (itIndex != possibleIndexes.end() && itValue != possibleValues.end())
                    {
                        if (*itIndex == this->configValue)
                        {
                            Manager::Get()->SetValueListSelection(this->zwaveValueId, *itValue);
                            break;
                        }
                        ++itIndex;
                        ++itValue;
                    }
            }
            break;
        }

        default:
        {
            break;
        }
    }
}
