#include "muduo/base/CurrentThread.h"
#include "muduo/base/Exception.h"
#include <functional>
#include <stdio.h>
#include <vector>

class Bar {
public:
    void test(std::vector<std::string> names = {}) {
        printf("Stack:\n%s\n", muduo::CurrentThread::stackTrace(true).c_str());
        [] {
            printf("Stack inside lambda:\n%s\n",
                   muduo::CurrentThread::stackTrace(true).c_str());
        }();
        std::function<void()> func([] {
            printf("Stack inside std::function:\n%s\n",
                   muduo::CurrentThread::stackTrace(true).c_str());
        });
        func();

        func = std::bind(&Bar::callback, this);
        func();

        throw muduo::Exception("oops");
    }

private:
    void callback() {
        printf("Stack inside std::bind:\n%s\n",
               muduo::CurrentThread::stackTrace(true).c_str());
    }
};

void foo() {
    Bar b;
    b.test();
}

/**
 * @note c++ 作 name mangling
 */
int main() {
    try {
        foo();
    } catch (const muduo::Exception &ex) {
        printf("reason: %s\n", ex.what());
        printf("stack trace:\n%s\n", ex.stackTrace());
    }
}
