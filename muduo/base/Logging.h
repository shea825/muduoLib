// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_LOGGING_H
#define MUDUO_BASE_LOGGING_H

#include "muduo/base/LogStream.h"
#include "muduo/base/Timestamp.h"

/**
 * @note
 * c++ 错误分类： 编译 运行 逻辑
 *
 * 日志作用：
 * 开发过程中： 调试错误 更好地理解程序
 * 运行过程中： 诊断系统故障并处理 记录系统运行状态
 *
 * 日志级别：
 * TRACE    指出比DEBUG粒度更细的一些信息事件（开发过程中使用）
 * DEBUG    指出细粒度信息事件（开发过程中使用）
 * INFO     表明信息在粗粒度级别上突出强调应用程序的运行过程
 * WARN     系统能正常运行，但可能会出现潜在的错误
 * ERROR    指出虽然发生错误事件，但仍然不影响系统的继续运行
 * FATAL    指出每个严重的错误事件将会导致应用程序的退出
 *
 * Logger 使用时序图
 * LogStream() = Logger.stream()
 * 调用LogStream对象的重载运算符 operator<<()实现日志输出
 * Logger() -> Impl() -> LogStream() -> operator<<() -> Buffer()    |
 * -> g_output() -> g_flush()                                       |
 *
 */
namespace muduo {

class TimeZone;

class Logger {
public:
  enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };

  // compile time calculation of basename of source file
  class SourceFile {
  public:
    template <int N>
    SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
      const char *slash = strrchr(data_, '/'); // builtin function
      if (slash) {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }

    explicit SourceFile(const char *filename) : data_(filename) {
      const char *slash = strrchr(filename, '/');
      if (slash) {
        data_ = slash + 1;
      }
      size_ = static_cast<int>(strlen(data_));
    }

    const char *data_;
    int size_;
  };

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char *func);
  Logger(SourceFile file, int line, bool toAbort);
  ~Logger();

  LogStream &stream() { return impl_.stream_; }

  static LogLevel logLevel();
  static void setLogLevel(LogLevel level);

  typedef void (*OutputFunc)(const char *msg, int len);
  typedef void (*FlushFunc)();
  static void setOutput(OutputFunc);
  static void setFlush(FlushFunc);
  static void setTimeZone(const TimeZone &tz);

private:
  /**
   * @note 嵌套类 logger的实际实现
   */
  class Impl {
  public:
    typedef Logger::LogLevel LogLevel;
    Impl(LogLevel level, int old_errno, const SourceFile &file, int line);
    void formatTime();
    void finish();

    Timestamp time_;
    LogStream stream_;
    LogLevel level_;
    int line_;
    SourceFile basename_;
  };

  Impl impl_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel() { return g_logLevel; }

//
// CAUTION: do not write:
//
// if (good)
//   LOG_INFO << "Good news";
// else
//   LOG_WARN << "Bad news";
//
// this expends to
//
// if (good)
//   if (logging_INFO)
//     logInfoStream << "Good news";
//   else
//     logWarnStream << "Bad news";
//
/**
 * @note 以下muduo::Logger()均为匿名临时对象    调用后立即析构
 */
#define LOG_TRACE                                                              \
  if (muduo::Logger::logLevel() <= muduo::Logger::TRACE)                       \
  muduo::Logger(__FILE__, __LINE__, muduo::Logger::TRACE, __func__).stream()
#define LOG_DEBUG                                                              \
  if (muduo::Logger::logLevel() <= muduo::Logger::DEBUG)                       \
  muduo::Logger(__FILE__, __LINE__, muduo::Logger::DEBUG, __func__).stream()
#define LOG_INFO                                                               \
  if (muduo::Logger::logLevel() <= muduo::Logger::INFO)                        \
  muduo::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN muduo::Logger(__FILE__, __LINE__, muduo::Logger::WARN).stream()
#define LOG_ERROR                                                              \
  muduo::Logger(__FILE__, __LINE__, muduo::Logger::ERROR).stream()
#define LOG_FATAL                                                              \
  muduo::Logger(__FILE__, __LINE__, muduo::Logger::FATAL).stream()
#define LOG_SYSERR muduo::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL muduo::Logger(__FILE__, __LINE__, true).stream()

const char *strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val)                                                     \
  ::muduo::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL",     \
                        (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T *CheckNotNull(Logger::SourceFile file, int line, const char *names, T *ptr) {
  if (ptr == NULL) {
    Logger(file, line, Logger::FATAL).stream() << names;
  }
  return ptr;
}

} // namespace muduo

#endif // MUDUO_BASE_LOGGING_H
