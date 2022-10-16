// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_CURRENTTHREAD_H
#define MUDUO_BASE_CURRENTTHREAD_H

#include "muduo/base/Types.h"

namespace muduo {
    namespace CurrentThread {
        // internal
        extern __thread int t_cachedTid;            //线程真实pid(tid)的缓存，减少syscall次数，提高效率
        extern __thread char t_tidString[32];       //tid的字符串表示形式
        extern __thread int t_tidStringLength;      //tid的字符串表示形式实际长度
        extern __thread const char *t_threadName;
        /**
         * @note __thread 修饰的变量是线程局部存储的 每个线程都有
         * __thread 只能修饰 POD类型（与C兼容的原始数据）
         * 线程特定数据 tsd 可用于 非POD类型
         */

//        __thread string t_obj1("cppcourse");  //F，不能调用对象的构造函数
//        __thread string* t_obj2 = new string; //F，初始化只能是编译器常量
//        __thread string* t_obj3 = nullptr;    //T
        void cacheTid();

        inline int tid() {
            if (__builtin_expect(t_cachedTid == 0, 0)) {
                cacheTid();
            }
            return t_cachedTid;
        }

        inline const char *tidString() // for logging
        {
            return t_tidString;
        }

        inline int tidStringLength() // for logging
        {
            return t_tidStringLength;
        }

        inline const char *name() {
            return t_threadName;
        }

        bool isMainThread();

        void sleepUsec(int64_t usec);  // for testing

        string stackTrace(bool demangle);
    }  // namespace CurrentThread
}  // namespace muduo

#endif  // MUDUO_BASE_CURRENTTHREAD_H
