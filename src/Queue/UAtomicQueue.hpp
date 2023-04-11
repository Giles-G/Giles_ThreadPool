/***************************
@File: UAtomicQueue.h
@Desc: 设计了一个安全队列
***************************/

#ifndef UATOMICQUEUE_H
#define UATOMICQUEUE_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

#include "../ThreadPoolinc.hpp"
#include "../CStdEx.hpp"

template <typename T>
class UAtomicQueue {
   public:
    UAtomicQueue() = default;

    /**
     * 等待弹出
     * @param value
     */
    CVoid waitPop(T& value) {
        UNIQUE_LOCK lk(mutex_);
        cv_.wait(lk, [this] { return !queue_.empty(); });
        value = std::move(*queue_.front());
        queue_.pop();
    }

    /**
     * 尝试弹出
     * @param value
     * @return
     */
    CBool tryPop(T& value) {
        LOCK_GUARD lk(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = std::move(*queue_.front());
        queue_.pop();
        return true;
    }

    /**
     * 尝试弹出多个任务
     * @param values
     * @param maxPoolBatchSize
     * @return
     */
    CBool tryPop(std::vector<T>& values, int maxPoolBatchSize) {
        LOCK_GUARD lk(mutex_);
        if (queue_.empty() || maxPoolBatchSize <= 0) {
            return false;
        }

        while (!queue_.empty() && maxPoolBatchSize--) {
            values.emplace_back(std::move(*queue_.front()));
            queue_.pop();
        }

        return true;
    }

    /**
     * 阻塞式等待弹出
     * @return
     */
    std::unique_ptr<T> waitPop() {
        UNIQUE_LOCK lk(mutex_);
        cv_.wait(lk, [this] { return !queue_.empty(); });
        std::unique_ptr<T> result = std::move(queue_.front());
        queue_.pop();
        return result;
    }

    /**
     * 非阻塞式等待弹出
     * @return
     */
    std::unique_ptr<T> tryPop() {
        LOCK_GUARD lk(mutex_);
        if (queue_.empty()) {
            return std::unique_ptr<T>();
        }
        std::unique_ptr<T> ptr = std::move(queue_.front());
        queue_.pop();
        return ptr;
    }

    /**
     * 传入数据
     * @param value
     */
    CVoid push(T&& value) {
        std::unique_ptr<T> task(c_make_unique<T>(std::move(value)));
        LOCK_GUARD lk(mutex_);
        queue_.push(std::move(task));
        cv_.notify_one();
    }

    /**
     * 判定队列是否为空
     * @return
     */
    [[nodiscard]] CBool empty() {
        LOCK_GUARD lk(mutex_);
        return queue_.empty();
    }

    NO_ALLOWED_COPY(UAtomicQueue)

   private:
    std::queue<std::unique_ptr<T>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

#endif  // UATOMICQUEUE_H
