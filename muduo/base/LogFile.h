// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_LOGFILE_H
#define MUDUO_BASE_LOGFILE_H

#include "muduo/base/Mutex.h"
#include "muduo/base/Types.h"

#include <memory>

namespace muduo {

namespace FileUtil {
class AppendFile;
}

class LogFile : noncopyable {
public:
  LogFile(const string &basename, off_t rollSize, bool threadSafe = true,
          int flushInterval = 3, int checkEveryN = 1024);
  ~LogFile();

  void append(const char *logline, int len);
  void flush();
  bool rollFile();

private:
  void append_unlocked(const char *logline, int len);

  static string getLogFileName(const string &basename, time_t *now);

  const string basename_;
  const off_t rollSize_;    //日志文件 达到 rollsize 换一个新文件
  const int flushInterval_; //日志写入 间隔时间
  const int checkEveryN_;

  int count_;

  std::unique_ptr<MutexLock> mutex_;
  time_t startOfPeriod_;    //开始记录日志时间（调整至 零点时间）
  time_t lastRoll_;         //上一次滚动日志文件时间
  time_t lastFlush_;        //上一次日志写入文件时间
  std::unique_ptr<FileUtil::AppendFile> file_;

  const static int kRollPerSeconds_ = 60 * 60 * 24;
};

} // namespace muduo
#endif // MUDUO_BASE_LOGFILE_H
