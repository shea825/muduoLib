//
// Created by shea on 10/9/22.
//
#include "muduo/net/EventLoop.h"
#include "muduo/net/Channel.h"
#include "muduo/base/Logging.h"

#include <cstdio>
#include <sys/timerfd.h>

using namespace muduo;
using namespace muduo::net;

EventLoop* g_loop;
int timerfd;

void timerout(Timestamp receiveTime) {
    printf("timerout\n");
    uint64_t howmany;
    ::read(timerfd, &howmany, sizeof(howmany));
    g_loop->quit();
}

int main() {
    Logger::setLogLevel(Logger::TRACE);
    EventLoop loop;
    g_loop = &loop;

    timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timerfd);
    channel.setReadCallback(timerout);

    channel.enableReading();

    struct itimerspec howlong{};
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 1;
    ::timerfd_settime(timerfd, 0, &howlong, nullptr);

    loop.loop();

    ::close(timerfd);
}
