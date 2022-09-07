#include "muduo/base/CurrentThread.h"
#include "muduo/base/Singleton.h"
#include "muduo/base/Thread.h"
#include "muduo/base/ThreadLocal.h"

#include <stdio.h>
#include <unistd.h>

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

#define STL muduo::Singleton<muduo::ThreadLocal<Test>>::instance().value()
/**
 * @note muduo::Singleton<muduo::ThreadLocal<Test>>::instance() 是单例
 * .value() 是线程特定数据
 * 通过 宏STL， 不同线程可访问同一个单例的数据 value() 此时value() Test类型
 * 但不同线程的访问到的value() 不是同一个对象
 */

void print() {
  printf("tid=%d, %p name=%s\n", muduo::CurrentThread::tid(), &STL,
         STL.name().c_str());
}

void threadFunc(const char *changeTo) {
  print();
  STL.setName(changeTo);
  sleep(1);
  print();
}

int main() {
  printf("主线程 tid=%d\n", muduo::CurrentThread::tid());
  STL.setName("main one");
  muduo::Thread t1(std::bind(threadFunc, "thread1"));
  muduo::Thread t2(std::bind(threadFunc, "thread2"));
  t1.start();
  t2.start();
  t1.join();
  print();
  t2.join();
  pthread_exit(nullptr);
}
