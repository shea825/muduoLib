#include "muduo/base/FileUtil.h"

#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <iostream>

using namespace muduo;

int main()
{

  string result;
  int64_t size = 0;
  int err = FileUtil::readFile("/proc/self", 1024, &result, &size);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::readFile("/proc/self", 1024, &result, NULL);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::readFile("/proc/self/cmdline", 1024, &result, &size);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::readFile("/dev/null", 1024, &result, &size);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::readFile("/dev/zero", 1024, &result, &size);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::readFile("/notexist", 1024, &result, &size);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::readFile("/dev/zero", 102400, &result, &size);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  err = FileUtil::readFile("/dev/zero", 102400, &result, NULL);
  printf("%d %zd %" PRIu64 "\n", err, result.size(), size);
  /**
   * @note PRIu64 宏 include in <inttypes.h>
   * 用于 解决 32位 和 64位机器 uint64_t 类型不同的问题
   *
   * 64位 uint64_t 是 long unsigned int 类型，请使用 "%ld"
   * 32位 uint64_t 是 long long unsigned int 类型，请使用 "%lld"
   */

  string line =
      "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  FileUtil::AppendFile af("/tmp/muduo_FileUtil_test");
  af.append(line.data(), line.size());
  af.flush();

  int64_t ssize = 0;
  int64_t ct, mt;

  FileUtil::ReadSmallFile sf("/tmp/muduo_FileUtil_test");
  string content;
  content.resize(FileUtil::ReadSmallFile::kBufferSize);
  sf.readToString<string>(FileUtil::ReadSmallFile::kBufferSize,
                               &content,
                               &ssize,
                               &mt,
                               &ct);
  std::cout << content << std::endl;


}

