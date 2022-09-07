#include "muduo/base/CurrentThread.h"
#include "muduo/base/Thread.h"
#include "muduo/base/ThreadLocalSingleton.h"

#include <stdio.h>

class Test : muduo::noncopyable {
public:
  Test() {
    printf("tid=%d, constructing %p\n", muduo::CurrentThread::tid(), this);
  }

  ~Test() {
    printf("tid=%d, destructing %p %s\n", muduo::CurrentThread::tid(), this,
           name_.c_str());
  }

  const muduo::string &name() const { return name_; }
  void setName(const muduo::string &n) { name_ = n; }

private:
  muduo::string name_;
};

void threadFunc(const char *changeTo) {
  printf("tid=%d, %p name=%s\n", muduo::CurrentThread::tid(),
         &muduo::ThreadLocalSingleton<Test>::instance(),
         muduo::ThreadLocalSingleton<Test>::instance().name().c_str());
  muduo::ThreadLocalSingleton<Test>::instance().setName(changeTo);
  printf("tid=%d, %p name=%s\n", muduo::CurrentThread::tid(),
         &muduo::ThreadLocalSingleton<Test>::instance(),
         muduo::ThreadLocalSingleton<Test>::instance().name().c_str());

  // no need to manually delete it
  // muduo::ThreadLocalSingleton<Test>::destroy();
}

int main() {
  printf("主线程tid=%d\n", muduo::CurrentThread::tid());
  //每个线程都有一个单例对象，类型为Test
  muduo::ThreadLocalSingleton<Test>::instance().setName("main one");
  printf("=============\n");
  muduo::Thread t1(std::bind(threadFunc, "thread1"));
  muduo::Thread t2(std::bind(threadFunc, "thread2"));
  t1.start();
  t2.start();
  t1.join();
  printf("=============\n");
  printf("tid=%d, %p name=%s\n", muduo::CurrentThread::tid(),
         &muduo::ThreadLocalSingleton<Test>::instance(),
         muduo::ThreadLocalSingleton<Test>::instance().name().c_str());
  printf("=============\n");
  t2.join();

  pthread_exit(nullptr);
}
