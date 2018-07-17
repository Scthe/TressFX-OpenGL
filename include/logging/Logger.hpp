#pragma once

#include <string>

// NOTE: NOT THREAD SAFE!
// NOTE: written long, long time ago, very crappy!

#define MIN_LOG_LEVEL logger::Trace

#define LOG(lvl)\
    if (lvl < MIN_LOG_LEVEL){}\
    else if(lvl < logger::Log::ReportingLevel) {} \
    else logger::Log().get( __FILE__, __LINE__, lvl)
    // else logger::Log().get( __FILE__, __LINE__, lvl).get_stream()

#define LOGT LOG(logger::Trace)
#define LOGD LOG(logger::Debug)
#define LOGI LOG(logger::Info)
#define LOGW LOG(logger::Warning)
#define LOGE LOG(logger::Error)

namespace logger {

  class LogImpl;

  enum LogLevel {
    Trace	  = 0,
    Debug	  = 1,
    Info	  = 2,
    Warning = 3,
    Error	  = 4
  };

  class Log {
    public:
    static LogLevel ReportingLevel;

    Log();
    ~Log();

    LogImpl& get(const char* const file, const int line, LogLevel level);

    protected:
    LogImpl* impl; // ugh, PIMPL logger! TODO placement new?

    private:
    Log(const Log&);
    Log& operator = (const Log&);
  };

  // template <typename T>
  // LogImpl& operator <<(LogImpl& log, T const& value) {
  //     log.your_stringstream << value;
  //     return log;
  // }

}

logger::LogImpl& operator<< (logger::LogImpl&, bool);
logger::LogImpl& operator<< (logger::LogImpl&, short);
logger::LogImpl& operator<< (logger::LogImpl&, int);
logger::LogImpl& operator<< (logger::LogImpl&, long);
logger::LogImpl& operator<< (logger::LogImpl&, unsigned long long);
logger::LogImpl& operator<< (logger::LogImpl&, unsigned short);
logger::LogImpl& operator<< (logger::LogImpl&, unsigned int);
logger::LogImpl& operator<< (logger::LogImpl&, unsigned long);
logger::LogImpl& operator<< (logger::LogImpl&, float);
logger::LogImpl& operator<< (logger::LogImpl&, double);
logger::LogImpl& operator<< (logger::LogImpl&, char*);
logger::LogImpl& operator<< (logger::LogImpl&, const char*);
logger::LogImpl& operator<< (logger::LogImpl&, std::string);
