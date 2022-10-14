// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include <set>
#include <vector>

#include "muduo/base/Mutex.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/Channel.h"

namespace muduo {
    namespace net {

        class EventLoop;

        class Timer;

        class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
        class TimerQueue : noncopyable {
        public:
            explicit TimerQueue(EventLoop *loop);

            ~TimerQueue();

            ///
            /// Schedules the callback to be run at given time,
            /// repeats if @c interval > 0.0.
            ///
            /// Must be thread safe. Usually be called from other threads.
            /// 通常不在所属eventloop对象的线程当中调用
            TimerId addTimer(TimerCallback cb,
                             Timestamp when,
                             double interval);

            void cancel(TimerId timerId);

        private:

            // FIXME: use unique_ptr<Timer> instead of raw pointers.
            // This requires heterogeneous comparison lookup (N3465) from C++14
            // so that we can find an T* in a set<unique_ptr<T>>.
            // TimerList和ActiveTimerSet保存相同的Timer列表，前者按到期时间排序，后者按对象地址排序
            typedef std::pair<Timestamp, Timer *> Entry;
            typedef std::set<Entry> TimerList;
            typedef std::pair<Timer *, int64_t> ActiveTimer;
            typedef std::set<ActiveTimer> ActiveTimerSet;

            //以下成员函数只可能在其所属的IO线程中调用，因而不必加锁

            void addTimerInLoop(Timer *timer);

            void cancelInLoop(TimerId timerId);

            // called when timerfd alarms
            void handleRead();

            // move out all expired timers
            // 返回超时的定时器列表
            std::vector<Entry> getExpired(Timestamp now);

            // 重置超时的定时器
            void reset(const std::vector<Entry> &expired, Timestamp now);

            bool insert(Timer *timer);

            EventLoop *loop_;                       //所属eventloop
            const int timerfd_;
            Channel timerfdChannel_;
            // Timer list sorted by expiration
            TimerList timers_;                      //按到期时间排序

            // for cancel()
            ActiveTimerSet activeTimers_;           //按对象地址排序
            bool callingExpiredTimers_; /* atomic */
            ActiveTimerSet cancelingTimers_;        //被取消的定时器
        };

    }  // namespace net
}  // namespace muduo
#endif  // MUDUO_NET_TIMERQUEUE_H
