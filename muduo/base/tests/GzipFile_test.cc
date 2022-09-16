#include "muduo/base/GzipFile.h"

#include "muduo/base/Logging.h"

int main() {
  const char *filename = "/tmp/gzipfile_test.gz";
  ::unlink(filename);
  const char data[] =
      "123456789012345678901234567890123456789012345678901234567890\n";
  off_t dataLen = strlen(data);
  {
    muduo::GzipFile writer = muduo::GzipFile::openForAppend(filename);
    if (writer.valid()) {
      LOG_INFO << "tell " << writer.tell();
      LOG_INFO << "wrote " << writer.write(data);
      LOG_INFO << "tell " << writer.tell();
      assert(dataLen == writer.tell());
    }
  }

  {
    printf("testing reader\n");
    muduo::GzipFile reader = muduo::GzipFile::openForRead(filename);
    if (reader.valid()) {
      char buf[256];
      LOG_INFO << "tell " << reader.tell();
      int nr = reader.read(buf, sizeof buf);
      printf("read %d\n", nr);
      if (nr >= 0) {
        buf[nr] = '\0';
        printf("data %s", buf);
      }
      LOG_INFO << "tell " << reader.tell();
      if (strncmp(buf, data, strlen(data)) != 0) {
        printf("failed!!!\n");
        abort();
      } else {
        printf("PASSED\n");
      }
    }
  }

  {
    muduo::GzipFile writer = muduo::GzipFile::openForWriteExclusive(filename);
    LOG_INFO << "valid " << writer.valid();

    LOG_INFO << "write "
             << (writer.write("123") == -1 ? strerror(errno) : "success");
    LOG_INFO << "tell " << writer.tell();
    LOG_INFO << "offset " << writer.offset();
    writer.~GzipFile();

    if (writer.valid() || errno != EEXIST) {
      printf("FAILED\n");
    }
  }

  {
    muduo::GzipFile reader = muduo::GzipFile::openForRead(filename);
    char buf[4];
    int bufLen = sizeof(buf);
    memset(buf, 0, bufLen);
    LOG_INFO << "read "
             << (reader.read(buf, bufLen) == -1 ? strerror(errno) : "success");
    buf[bufLen - 1] = '\0';
    printf("data %s\n", buf);
    LOG_INFO << "tell " << reader.tell();
    LOG_INFO << "offset " << reader.offset();
  }
}
