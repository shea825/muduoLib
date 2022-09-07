#include "muduo/base/CurrentThread.h"
#include "muduo/base/Thread.h"
#include "muduo/base/ThreadLocal.h"

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

muduo::ThreadLocal<Test> testObj1;
muduo::ThreadLocal<Test> testObj2;

void print() {
  printf("======================================================\n");
  printf("tid=%d, obj1 %p name=%s\n", muduo::CurrentThread::tid(),
         &testObj1.value(), testObj1.value().name().c_str());
  printf("tid=%d, obj2 %p name=%s\n", muduo::CurrentThread::tid(),
         &testObj2.value(), testObj2.value().name().c_str());
  printf("======================================================\n");
}

void threadFunc() {
  print();
  testObj1.value().setName("changed 1");
  testObj2.value().setName("changed 42");
  print();
}

int main() {
  testObj1.value().setName("main one");
  print();
  muduo::Thread t1(threadFunc); //创建子线程，线程特定数据没有，name应当为空
  t1.start();
  t1.join();
  testObj2.value().setName("main two");
  print();

  pthread_exit(nullptr);
}
