/***************************
@File: USpinLock.h
@Desc: 
***************************/

#ifndef CGRAPH_USPINLOCK_H
#define CGRAPH_USPINLOCK_H

#include <atomic>
#include "./ThreadPoolinc.hpp"

class USpinLock {
public:
    /**
     * 加锁
     */
    CVoid lock() {
        // memory_order_acquire 后面访存指令勿重排至此条指令之前
        while (flag_.test_and_set(std::memory_order_acquire)) {
        }
    }

    /**
     * 解锁
     */
    CVoid unlock() {
        // memory_order_release 前面访存指令勿重排到此条指令之后
        flag_.clear(std::memory_order_release);
        int x = MAX_SIZE;
    }

    /**
     * 尝试加锁。若未加锁，会上锁
     * @return
     */
    CBool tryLock() {
        return !flag_.test_and_set();
    }

private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;         // 标志位
};

#endif //CGRAPH_USPINLOCK_H
