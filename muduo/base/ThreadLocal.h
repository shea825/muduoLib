// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREADLOCAL_H
#define MUDUO_BASE_THREADLOCAL_H

#include "muduo/base/Mutex.h"
#include "muduo/base/noncopyable.h"

#include <pthread.h>
/**
 * @note 在单线程程序中，经常用到“全局变量”以实现多个函数间共享数据
 *
 * 在多线程环境中，由于数据空间是共享的，因此全局变量也为所有线程所共有
 *
 * 但有时应用程序设计中有必要提供线程私有的全局变量，仅在某个线程中有效，
 * 但却可以跨多个函数访问
 *
 * POSIX线程库通过维护一定的数据结构来解决这个问题，这些数据结构称为
 * Thread-specific-data(TSD)，也称作线程本地存储(TLS)
 *
 * 对于POD类型的线程本地存储，可以用 __thread关键字
 *
 * pthread_key_create()
 * pthread_key_delete()
 *
 * pthread_setspecific()
 * pthread_getspecific()
 */
namespace muduo {

template <typename T> class ThreadLocal : noncopyable {
public:
  ThreadLocal() {
    MCHECK(pthread_key_create(&pkey_, &ThreadLocal::destructor));
  }

  ~ThreadLocal() { MCHECK(pthread_key_delete(pkey_)); }

  T &value() {
    T *perThreadValue = static_cast<T *>(pthread_getspecific(pkey_)); //获取特定数据
    if (!perThreadValue) {
      T *newObj = new T();
      MCHECK(pthread_setspecific(pkey_, newObj));
      perThreadValue = newObj;
    }
    return *perThreadValue;
  }

private:
  /**
   * @brief pthread_key_create() 传入所需
   * 回调函数销毁 实际数据
   * @param x
   */
  static void destructor(void *x) {
    T *obj = static_cast<T *>(x);
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
    T_must_be_complete_type dummy;
    (void)dummy;
    delete obj;
  }

private:
  pthread_key_t pkey_{};
};

} // namespace muduo

#endif // MUDUO_BASE_THREADLOCAL_H
