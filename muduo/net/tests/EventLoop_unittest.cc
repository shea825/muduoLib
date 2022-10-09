#include "muduo/base/Thread.h"
#include "muduo/net/EventLoop.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;
#define GLOBAL_LOOP 0
EventLoop *g_loop;

void callback() {
  printf("callback(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  EventLoop anotherLoop;
}

void threadFunc() {
  printf("threadFunc(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());

  assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
#if GLOBAL_LOOP
  g_loop->loop();
#else
  EventLoop loop;
  assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
  loop.runAfter(1.0, callback);
  loop.loop();
#endif
}

int main() {
  printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());

  assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
  EventLoop loop;
  assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
#if GLOBAL_LOOP
  g_loop = &loop;
#endif
  Thread thread(threadFunc);
  thread.start();

  loop.loop();
  thread.join();

  return EXIT_SUCCESS;
}
