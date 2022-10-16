// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "muduo/net/TimerQueue.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/Timer.h"
#include "muduo/net/TimerId.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace muduo {
    namespace net {
        namespace detail {

            int createTimerfd() {
                int timerfd = ::timerfd_create(CLOCK_MONOTONIC,   //不受不连续系统时间更改的影响,跟实际时间有关
                                               TFD_NONBLOCK | TFD_CLOEXEC);
                if (timerfd < 0) {
                    LOG_SYSFATAL << "Failed in timerfd_create";
                }
                return timerfd;
            }

            struct timespec howMuchTimeFromNow(Timestamp when) {
                int64_t microseconds = when.microSecondsSinceEpoch()
                                       - Timestamp::now().microSecondsSinceEpoch();
                if (microseconds < 100) {
                    microseconds = 100;
                }
                struct timespec ts;
                ts.tv_sec = static_cast<time_t>(
                        microseconds / Timestamp::kMicroSecondsPerSecond);
                ts.tv_nsec = static_cast<long>(
                        (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
                return ts;
            }

            void readTimerfd(int timerfd, Timestamp now) {
                uint64_t howmany;
                ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
                LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
                if (n != sizeof howmany) {
                    LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
                }
            }

            void resetTimerfd(int timerfd, Timestamp expiration) {
                // wake up loop by timerfd_settime()
                struct itimerspec newValue;
                struct itimerspec oldValue;
                memZero(&newValue, sizeof newValue);
                memZero(&oldValue, sizeof oldValue);
                newValue.it_value = howMuchTimeFromNow(expiration);
                int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
                if (ret) {
                    LOG_SYSERR << "timerfd_settime()";
                }
            }

        }  // namespace detail
    }  // namespace net
}  // namespace muduo

using namespace muduo;
using namespace muduo::net;
using namespace muduo::net::detail;

TimerQueue::TimerQueue(EventLoop *loop)
        : loop_(loop),
          timerfd_(createTimerfd()),
          timerfdChannel_(loop, timerfd_),
          timers_(),
          callingExpiredTimers_(false) {
    timerfdChannel_.setReadCallback(    //当定时器通道可读事件产生的时候，会回调handleRead
            std::bind(&TimerQueue::handleRead, this));
    // we are always reading the timerfd, we disarm it with timerfd_settime.
    timerfdChannel_.enableReading();    //这个通道加入poll并被关注
}

TimerQueue::~TimerQueue() {
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
    // do not remove channel, since we're in EventLoop::dtor();
    for (const Entry &timer : timers_) {
        delete timer.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb,
                             Timestamp when,
                             double interval) {
    Timer *timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(
            std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop(
            std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer *timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);               //插入一个定时器，可能会使最早到期的定时器发生改变

    if (earliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());    //重置定时器的超时时刻
    }
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if (it != activeTimers_.end()) {        //找到了定时器所对应的timerfd
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1);
        (void) n;
        delete it->first; // FIXME: no delete please
        activeTimers_.erase(it);
    } else if (callingExpiredTimers_) {     //定义器已经到期
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);                     //清除该事件，避免一直触发

    std::vector<Entry> expired = getExpired(now);   //获取该时刻之前所有的定时器列表

    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    // safe to callback outside critical section
    for (const Entry &it : expired) {
        it.second->run();                           //回调定时器处理函数
    }
    callingExpiredTimers_ = false;

    reset(expired, now);                            //不是一次性定时器，需要重启
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    TimerList::iterator end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || now < end->first);
    std::copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    for (const Entry &it : expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n == 1);
        (void) n;
    }

    assert(timers_.size() == activeTimers_.size());
    return expired;
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
    Timestamp nextExpire;

    for (const Entry &it : expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        if (it.second->repeat()     //如果是重复的定时器并且是未取消定时器，则重启该定时器
            && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            it.second->restart(now);
            insert(it.second);
        } else {                    //一次性定时器或者已被取消的定时器是不能重置的，因此删除该定时器
            // FIXME move to a free list
            delete it.second; // FIXME: no delete please
        }
    }

    if (!timers_.empty()) {
        nextExpire = timers_.begin()->second->expiration();
    }

    if (nextExpire.valid()) {
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(Timer *timer) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;   //最早到期时间是否发生改变
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }
    {
        std::pair<TimerList::iterator, bool> result
                = timers_.insert(Entry(when, timer));
        assert(result.second);
        (void) result;
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result
                = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second);
        (void) result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

