#ifndef UTHREADPRIMARY_H
#define UTHREADPRIMARY_H

#include "./UThreadBase.hpp"
#include "../UtilsDefine.hpp"
#include "../CFuncType.hpp"

class UThreadPrimary : public UThreadBase {
protected:
    explicit UThreadPrimary() {
        index_ = SECONDARY_THREAD_COMMON_ID;
        pool_threads_ = nullptr;
        type_ = THREAD_TYPE_PRIMARY;
    }


    CStatus init() {
        FUNCTION_BEGIN
        ASSERT_INIT(false)

        is_init_ = true;
        thread_ = std::move(std::thread(&UThreadPrimary::run, this));
        setSchedParam();
        setAffinity(index_);
        FUNCTION_END
    }


    /**
     * 注册线程池相关内容，需要在init之前使用
     * @param index
     * @param poolTaskQueue
     * @param poolThreads
     * @param config
     */
    CStatus setThreadPoolInfo(int index,
                              UAtomicQueue<UTask>* poolTaskQueue,
                              std::vector<UThreadPrimary *>* poolThreads,
                              UThreadPoolConfigPtr config) {
        FUNCTION_BEGIN
        ASSERT_INIT(false)    // 初始化之前，设置参数
        ASSERT_NOT_NULL(poolTaskQueue)
        ASSERT_NOT_NULL(poolThreads)
        ASSERT_NOT_NULL(config)

        this->index_ = index;
        this->pool_task_queue_ = poolTaskQueue;
        this->pool_threads_ = poolThreads;
        this->config_ = config;
        FUNCTION_END
    }


    /**
     * 线程执行函数
     * @return
     */
    CStatus run() {
        FUNCTION_BEGIN
        ASSERT_INIT(true)
        ASSERT_NOT_NULL(pool_threads_)
        ASSERT_NOT_NULL(config_)

        /**
         * 线程池中任何一个primary线程为null都不可以执行
         * 防止线程初始化失败的情况，导致的崩溃
         * 理论不会走到这个判断逻辑里面
         */
        if (std::any_of(pool_threads_->begin(), pool_threads_->end(),
                        [](UThreadPrimary* thd) {
                            return nullptr == thd;
                        })) {
            RETURN_ERROR_STATUS("primary thread is null")
        }

        if (config_->calcBatchTaskRatio()) {
            while (done_) {
                processTasks();    // 批量任务获取执行接口
            }
        } else {
            while (done_) {
                processTask();    // 单个任务获取执行接口
            }
        }

        FUNCTION_END
    }


    /**
     * 获取并执行任务
     * @return
     */
    CVoid processTask() {
        UTask task;
        if (popTask(task) || popPoolTask(task) || stealTask(task)) {
            runTask(task);
        } else {
            std::this_thread::yield(); // 没有任务就不要阻塞，让出cpu
        }
    }


    /**
     * 获取批量执行task信息
     */
    CVoid processTasks() {
        UTaskArr tasks;
        if (popTask(tasks) || popPoolTask(tasks) || stealTask(tasks)) {
            // 尝试从主线程中获取/盗取批量task，如果成功，则依次执行
            runTasks(tasks);
        } else {
            std::this_thread::yield();
        }
    }


    /**
     * 从本地弹出一个任务
     * @param task
     * @return
     */
    bool popTask(UTaskRef task) {
        return work_stealing_queue_.tryPop(task);
    }


    /**
     * 从本地弹出一批任务
     * @param tasks
     * @return
     */
    bool popTask(UTaskArrRef tasks) {
        return work_stealing_queue_.tryPop(tasks, config_->max_local_batch_size_);
    }


    /**
     * 从其他线程窃取一个任务
     * @param task
     * @return
     */
    bool stealTask(UTaskRef task) {
        if (unlikely(pool_threads_->size() < config_->default_thread_size_)) {
            /**
             * 线程池还未初始化完毕的时候，无法进行steal。
             * 确保程序安全运行。
             */
            return false;
        }

        /**
         * 窃取的时候，仅从相邻的primary线程中窃取
         * 待窃取相邻的数量，不能超过默认primary线程数
         */
        int range = config_->calcStealRange();
        for (int i = 0; i < range; i++) {
            /**
            * 从线程中周围的thread中，窃取任务。
            * 如果成功，则返回true，并且执行任务。
            */
            int curIndex = (index_ + i + 1) % config_->default_thread_size_;
            if (nullptr != (*pool_threads_)[curIndex]
                && ((*pool_threads_)[curIndex])->work_stealing_queue_.trySteal(task)) {
                return true;
            }
        }

        return false;
    }


    /**
     * 从其他线程盗取一批任务
     * @param tasks
     * @return
     */
    bool stealTask(UTaskArrRef tasks) {
        if (unlikely(pool_threads_->size() < config_->default_thread_size_)) {
            return false;
        }

        int range = config_->calcStealRange();
        for (int i = 0; i < range; i++) {
            int curIndex = (index_ + i + 1) % config_->default_thread_size_;
            if (nullptr != (*pool_threads_)[curIndex]
                && ((*pool_threads_)[curIndex])->work_stealing_queue_.trySteal(tasks, config_->max_steal_batch_size_)) {
                return true;
            }
        }

        return false;
    }

private:
    int index_ {SECONDARY_THREAD_COMMON_ID};                // 线程index
    UWorkStealingQueue work_stealing_queue_;                       // 内部队列信息
    std::vector<UThreadPrimary *>* pool_threads_;                  // 用于存放线程池中的线程信息

    friend class UThreadPool;
    friend class UAllocator;
};

using UThreadPrimaryPtr = UThreadPrimary *;

#endif //UTHREADPRIMARY_H
