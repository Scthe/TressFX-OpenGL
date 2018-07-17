#include <ctime>
#include <cstring>
// #include <algorithm>

// Available patterns:
// - %h - hours
// - %m - minutes
// - %s - seconds
// - %p - log level (shortened to D/I/W/E)
// - %f - filename
// - %l - line
//
// e.g.
//  - "%h:%m:%s %f:%l [%p] "
//  - "%f:%l [%p] ";
logger::LogImpl::LogPattern logger::LogImpl::Pattern = "%f:%l [%p] ";

const char* level_symbol[] = {"T", "D","I","W","E"};

static logger::ColorFg_ get_color_by_level (logger::LogLevel);
static void set_color (std::ostringstream&, logger::ColorFg_);
static void add_symbol(char, std::ostringstream&, const logger::MsgMetaData&);

#define LOC_ACCURACY 5
#define FILENAME_LETTERS 25

namespace logger {


  struct MsgMetaData {
    logger::LogLevel level;
    const char* const file_name;
    int line;
    std::tm* time;
  };

  LogImpl& Log::get(const char* const file_name, const int line, LogLevel level){
    std::time_t t = std::time(0);
    impl->set_header({level, file_name, line, std::localtime(&t)});
    return *impl;
  }

  void LogImpl::set_header(const MsgMetaData& meta) {
    auto pattern = LogImpl::Pattern;

    // os << color_by_level(meta.level);
    set_color(this->os, get_color_by_level(meta.level));

    char* it = const_cast<char*>(pattern);
    while(*it != '\0'){
      if(*it == '%'){
        ++it;
        add_symbol(*it, os, meta);
      } else {
        os << *it;
      }

      ++it;
    }

    set_color(this->os, ColorFg::White);
  }

}

static char* get_filename(const char* const fullpath) {
  // extracts filename from path
  auto s1 = strrchr(fullpath, '/'),
       s2 = strrchr(fullpath, '\\');
  char* last_slash = const_cast<char*>(fullpath);

  if (s1 && s1 > s2) {
    last_slash = s1 + 1;
  } else if (s2) {
    last_slash = s2 + 1;
  }

  return last_slash;
}

static void put_exactly (std::ostringstream& os, char* str_to_add, unsigned int to_put) {
  auto len = strlen(str_to_add);
  if (len > to_put) {
    str_to_add += len - to_put;
  }

  os << str_to_add;

  for (size_t i = len; i < to_put; i++) {
    os << ' ';
  }
}

void add_symbol(char it, std::ostringstream& os, const logger::MsgMetaData& data){
  switch(it){
    case 'h':				// hour
      os << data.time->tm_hour;
      break;
    case 'm':				// minutes
      os << data.time->tm_min;
      break;
    case 's':				// seconds
      os << data.time->tm_sec;
      break;
    case 'p':				// log level
      os << level_symbol[data.level];
      break;
    case 'f':				// file
      // os << get_filename(data.file_name);
      put_exactly(os, get_filename(data.file_name), FILENAME_LETTERS);
      break;
    case 'l':{			// line number
      char pat[] = {'%', '0','d', '\0'};
      pat[1] += LOC_ACCURACY - 1;
      char buffer[LOC_ACCURACY+1];
      snprintf(buffer, LOC_ACCURACY, pat, data.line);
      os << buffer;
      break;}
    default:
      os << '%'; break;
  }
}

logger::ColorFg_ get_color_by_level (logger::LogLevel lvl) {
  switch(lvl) {
    case logger::LogLevel::Trace:   return logger::ColorFg::Magenta; // Cyan?
    case logger::LogLevel::Debug:   return logger::ColorFg::Green;
    case logger::LogLevel::Info:    return logger::ColorFg::Blue;
    case logger::LogLevel::Warning: return logger::ColorFg::Yellow;
    case logger::LogLevel::Error:   return logger::ColorFg::Red;
  }
}

static void set_color (std::ostringstream& os, logger::ColorFg_ col) {
  os << "\033[0;" << col << "m";
}
