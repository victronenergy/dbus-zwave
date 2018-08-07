#include <cstdarg>
#include <cstring>
#include <string>

extern "C" {
#include <velib/types/ve_str.h>
#include <velib/utils/ve_logger.h>
}

#include <Defs.h>
#include <platform/Log.h>

#include "dz_logger.hpp"

using OpenZWave::LogLevelString;
using OpenZWave::LogLevel;
using std::string;

DZLogger::DZLogger() {}
DZLogger::~DZLogger() {}

void DZLogger::Write(LogLevel level, const uint8 nodeId, const char* format, va_list args)
{
    if (format == NULL || format[0] == '\0')
    {
        return;
    }

    // Reduce startup message level
    if (level == LogLevel::LogLevel_Always && nodeId == 0 && strcmp(format, "OpenZwave Version %s Starting Up") == 0)
    {
        level = LogLevel::LogLevel_Info;
    }

    void (*logFunc)(char const* module, char const* format, ...);
    switch (level)
    {
		case LogLevel::LogLevel_Always:
		case LogLevel::LogLevel_Error:
		case LogLevel::LogLevel_Fatal:
        {
            logFunc = logE;
            break;
        }
		case LogLevel::LogLevel_Alert:
		case LogLevel::LogLevel_Warning:
        {
            logFunc = logW;
            break;
        }
		case LogLevel::LogLevel_Detail:
		case LogLevel::LogLevel_Info:
        {
            logFunc = logI;
            break;
        }
		case LogLevel::LogLevel_Debug:
		case LogLevel::LogLevel_Invalid:
		case LogLevel::LogLevel_None:
		case LogLevel::LogLevel_StreamDetail:
        case LogLevel::LogLevel_Internal:
        default:
        {
            logFunc = 0;
            break;
        }
    }

    if (logFunc != 0)
    {
        VeStr formattedLine;
        veStrNew(&formattedLine, 1024, 1024);
        if (nodeId == 0)
        {
            veStrAddFormat(&formattedLine, "%s: ", LogLevelString[level]);
        }
        else
        {
            veStrAddFormat(&formattedLine, "%s, node %d: ", LogLevelString[level], nodeId);
        }
        veStrAddFormatV(&formattedLine, format, args);
        logFunc("OpenZWave", "%s", veStrCStr(&formattedLine));
        veStrFree(&formattedLine);
    }
}

void DZLogger::QueueDump()
{
    //
}

void DZLogger::QueueClear()
{
    //
}

void DZLogger::SetLoggingState(LogLevel saveLevel, LogLevel queueLevel, LogLevel dumpTrigger)
{
    VE_UNUSED(saveLevel);
    VE_UNUSED(queueLevel);
    VE_UNUSED(dumpTrigger);
}

void DZLogger::SetLogFileName(const string& filename)
{
    VE_UNUSED(filename);
}
