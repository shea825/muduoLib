// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_COUNTDOWNLATCH_H
#define MUDUO_BASE_COUNTDOWNLATCH_H

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

namespace muduo {

    class CountDownLatch : noncopyable {
    public:

        explicit CountDownLatch(int count);

        void wait();

        void countDown();   //计数器减一

        int getCount() const;

    private:
        mutable MutexLock mutex_;
        /**
         * @note const 函数 可改变 mutable 修饰的 成员变量的 状态
         */
        Condition condition_ GUARDED_BY(mutex_);
        int count_ GUARDED_BY(mutex_);
    };

}  // namespace muduo
#endif  // MUDUO_BASE_COUNTDOWNLATCH_H
