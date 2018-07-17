#include "../../include/logging/Logger.hpp"
#include <sstream>
#include <iostream>

namespace logger {

  // default logging level
  LogLevel Log::ReportingLevel = LogLevel::Debug;

  struct MsgMetaData;

  struct ColorFg {
    static const int Black  = 30;
    static const int Red    = 31;
    static const int Green  = 32;
    static const int Yellow = 33;
    static const int Blue   = 34;
    static const int Magenta= 35;
    static const int Cyan   = 36;
    static const int White  = 37;
  };
  typedef int ColorFg_;

  class LogImpl {
    public:
    typedef const char* const LogPattern;
    static LogPattern Pattern;

    typedef std::ostringstream stream_t;

    void set_header(const MsgMetaData&);

    void flush () {
      auto str = this->os.str();
      std::cout << str << std::endl;
    }

    public:
    stream_t os;
  };

  Log::Log() : impl(new LogImpl){} // ugh! PIMPL

  Log::~Log(){
    this->impl->flush();
    delete this->impl;
  }

}

#include "formatter.impl.hpp"

using namespace logger;

LogImpl& operator<< (LogImpl& l, bool v) {
  const char* vals[] = {"True", "False"};
  const char* val_str = vals[v ? 0 : 1];
  l.os << (val_str);
  return l;
}
// signed
LogImpl& operator<< (LogImpl& l, short val){ return l << (long) val; }
LogImpl& operator<< (LogImpl& l, int val)  { return l << (long) val; }
LogImpl& operator<< (LogImpl& l, long val) {
  char buffer [128];
  snprintf(buffer, sizeof(buffer), "%ld", val);
  l.os << buffer;
  return l;
}
// unsigned
LogImpl& operator<< (LogImpl& l, unsigned short val){ return l << (unsigned long) val; }
LogImpl& operator<< (LogImpl& l, unsigned int val)  { return l << (unsigned long) val; }
LogImpl& operator<< (LogImpl& l, unsigned long long val)  { return l << (unsigned long) val; } // bwahahahahahahhahaaha!
LogImpl& operator<< (LogImpl& l, unsigned long val) {
  char buffer [128];
  snprintf(buffer, sizeof(buffer), "%lu", val);
  l.os << buffer;
  return l;
}
// floats
LogImpl& operator<< (LogImpl& l, float val) { return l << (double) val; }
LogImpl& operator<< (LogImpl& l, double val) {
  char buffer [128];
  snprintf(buffer, sizeof(buffer), "%f", val);
  l.os << buffer;
  return l;
}
// strings
LogImpl& operator<< (LogImpl& l, char* val) { return l << std::string(val); }
LogImpl& operator<< (LogImpl& l, const char* val) { return l << std::string(val); }
LogImpl& operator<< (LogImpl& l, std::string str) {
  // TODO handle '\0'
  l.os << str;
  return l;
}
