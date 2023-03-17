/***************************
@Author: Chunel
@Contact: chunel@foxmail.com
@File: UThreadSecondary.h
@Time: 2021/7/8 11:02 下午
@Desc: 
***************************/

#ifndef UTHREADSECONDARY_H
#define UTHREADSECONDARY_H

#include "./UThreadBase.hpp"
#include "../UtilsDefine.hpp"
#include "../CFuncType.hpp"

class UThreadSecondary : public UThreadBase {
public:
    explicit UThreadSecondary() {
        cur_ttl_ = 0;
        type_ = THREAD_TYPE_SECONDARY;
    }


protected:
    CStatus init() {
        FUNCTION_BEGIN
        ASSERT_INIT(false)
        ASSERT_NOT_NULL(config_)

        cur_ttl_ = config_->secondary_thread_ttl_;
        is_init_ = true;
        thread_ = std::move(std::thread(&UThreadSecondary::run, this));
        setSchedParam();
        FUNCTION_END
    }


    /**
     * 设置pool的信息
     * @param poolTaskQueue
     * @param poolPriorityTaskQueue
     * @param config
     * @return
     */
    CStatus setThreadPoolInfo(UAtomicQueue<UTask>* poolTaskQueue,
                              UAtomicPriorityQueue<UTask>* poolPriorityTaskQueue,
                              UThreadPoolConfigPtr config) {
        FUNCTION_BEGIN
        ASSERT_INIT(false)    // 初始化之前，设置参数
        ASSERT_NOT_NULL(poolTaskQueue)
        ASSERT_NOT_NULL(poolPriorityTaskQueue)
        ASSERT_NOT_NULL(config)

        this->pool_task_queue_ = poolTaskQueue;
        this->pool_priority_task_queue_ = poolPriorityTaskQueue;
        this->config_ = config;
        FUNCTION_END
    }


    CStatus run() {
        FUNCTION_BEGIN
        ASSERT_INIT(true)
        ASSERT_NOT_NULL(config_)

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
     * 任务执行函数，从线程池的任务队列中获取信息
     */
    CVoid processTask() {
        UTask task;
        if (popPoolTask(task)) {
            runTask(task);
        } else {
            std::this_thread::yield();
        }
    }


    /**
     * 批量执行n个任务
     */
    CVoid processTasks() {
        UTaskArr tasks;
        if (popPoolTask(tasks)) {
            runTasks(tasks);
        } else {
            std::this_thread::yield();
        }
    }


    /**
     * 判断本线程是否需要被自动释放
     * @return
     */
    bool freeze() {
        if (likely(is_running_)) {
            cur_ttl_++;
            cur_ttl_ = std::min(cur_ttl_, config_->secondary_thread_ttl_);
        } else {
            cur_ttl_--;    // 如果当前线程没有在执行，则ttl-1
        }

        return cur_ttl_ <= 0;
    }

private:
    int cur_ttl_ = 0;                                                      // 当前最大生存周期

    friend class UThreadPool;
};

using UThreadSecondaryPtr = UThreadSecondary *;

#endif // UTHREADSECONDARY_H
