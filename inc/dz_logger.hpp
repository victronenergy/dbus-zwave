#ifndef DZ_LOGGER_H
#define DZ_LOGGER_H

#include <string>
#include <cstdarg>

#include <Defs.h>
#include <platform/Log.h>

using OpenZWave::i_LogImpl;
using OpenZWave::LogLevel;
using std::string;

class DZLogger : public i_LogImpl {
  public:
    explicit DZLogger();
    virtual ~DZLogger();

    virtual void Write(LogLevel level, const uint8 nodeId, const char* format, va_list args) override;
    virtual void QueueDump() override;
    virtual void QueueClear() override;
    virtual void SetLoggingState(LogLevel saveLevel, LogLevel queueLevel, LogLevel dumpTrigger) override;
    virtual void SetLogFileName(const string& filename) override;
};

#endif
