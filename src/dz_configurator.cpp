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
#include "dz_util.hpp"

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
        if (DZUtil::match(zwaveValueId, c.first)) {
            return true;
        }
    }
    for (const auto& p : pollingMap)
    {
        if (DZUtil::match(zwaveValueId, p.first)) {
            return true;
        }
    }
    return false;
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
        if (DZUtil::match(zwaveValueId, c.first))
        {
            this->configValue = c.second;
            this->hasConfigValue = true;
            break;
        }
    }
    this->hasPollingIntensity = false;
    for (const auto& p : this->getPollingMap())
    {
        if (DZUtil::match(zwaveValueId, p.first))
        {
            this->pollingIntensity = p.second;
            this->hasPollingIntensity = true;
            break;
        }
    }
    if (awaitQueryComplete)
    {
        logI("DZConfigurator", "Delayed binding %s...", DZUtil::path(this->zwaveValueId).c_str());
        Manager::Get()->AddWatcher(DZConfigurator::onZwaveNotification, (void*) this);
    }
    else
    {
        logI("DZConfigurator", "Binding %s", DZUtil::path(this->zwaveValueId).c_str());
        this->initCompleted = true;
        this->lastUpdate = system_clock::now();
        Manager::Get()->AddWatcher(DZConfigurator::onZwaveNotification, (void*) this);
        this->update();
        if (this->hasPollingIntensity)
        {
            logI("DZConfigurator", "Setting polling intensity of %s to %d", DZUtil::path(this->zwaveValueId).c_str(), this->pollingIntensity);
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
                logI("DZConfigurator", "Binding %s", DZUtil::path(this->zwaveValueId).c_str());
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
                            this->update();
                        }
                        else
                        {
                            logI("DZConfigurator", "%s changed within cooldown, ignoring", DZUtil::path(this->zwaveValueId).c_str());
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
    if (this->hasConfigValue)
    {
        if (DZUtil::setZwaveValueByVariant(this->zwaveValueId, this->configValue))
        {
            this->lastUpdate = system_clock::now();
            logI("DZConfigurator", "Set %s to %d", DZUtil::path(this->zwaveValueId).c_str(), this->configValue);
        }
        else
        {
            logI("DZConfigurator", "Failed to set %s to %d", DZUtil::path(this->zwaveValueId).c_str(), this->configValue);
        }
    }
}
