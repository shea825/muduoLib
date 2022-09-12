#include "muduo/base/LogFile.h"
#include "muduo/base/Logging.h"

#include <functional>
#include <unistd.h>

std::unique_ptr<muduo::LogFile> g_logFile;
FILE *g_file;

void outputFunc(const char *msg, int len) { g_logFile->append(msg, len); }

void outputFunc1(const char *msg, int len) {
  if (g_file) {
    fwrite(msg, 1, len, g_file);
  }
}

void flushFunc() { g_logFile->flush(); }

void flushFunc1() {
  fflush(g_file);
}

int main(int argc, char *argv[]) {
#if 1
  /**
   * @note UTC 时间
   */
  char name[256] = {'\0'};
  strncpy(name, argv[0], sizeof name - 1);
  g_logFile.reset(new muduo::LogFile(::basename(name), 200 * 1000));
  muduo::Logger::setOutput(outputFunc);
  muduo::Logger::setFlush(flushFunc);

  muduo::string line =
      "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

  for (int i = 0; i < 10000; ++i) {
    LOG_INFO << line << i;

    usleep(1000);
  }
#else
  g_file = ::fopen("/tmp/muduo_log", "ae");
  muduo::Logger::setOutput(outputFunc1);
  muduo::Logger::setFlush(flushFunc1);

  LOG_TRACE << "trace";
  LOG_DEBUG << "debug";
  LOG_INFO << "info";
  LOG_WARN << "warn";
  LOG_ERROR << "error";

  ::fclose(g_file);
#endif
  return EXIT_SUCCESS;
}
