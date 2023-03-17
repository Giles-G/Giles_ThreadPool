#ifndef UTHREADBASE_HPP
#define UTHREADBASE_HPP

#include <thread>
#include <algorithm>

#include "../ThreadPoolinc.hpp"
#include "../Queue/UQueueInclude.hpp"
#include "../UThreadPoolConfig.hpp"
#include "../Task/UTask.hpp"
#include "../UtilsDefine.hpp"
#include "../UAllocator.hpp"


class UThreadBase : CObject{
protected:
    explicit UThreadBase() {
        done_ = true;
        is_init_ = false;
        is_running_ = false;
        pool_task_queue_ = nullptr;
        pool_priority_task_queue_ = nullptr;
        config_ = nullptr;
        total_task_num_ = 0;
    }


    ~UThreadBase() {
        reset();
    }


    /**
     * 所有线程类的 destroy 函数应该是一样的
     * 但是init函数不一样，因为线程构造函数不同
     * @return
     */
    CStatus destroy() {
        FUNCTION_BEGIN
        ASSERT_INIT(true)

        reset();
        FUNCTION_END
    }


    /**
     * 从线程池的队列中，获取任务
     * @param task
     * @return
     */
    virtual bool popPoolTask(UTaskRef task) {
        bool result = pool_task_queue_->tryPop(task);
        if (!result && THREAD_TYPE_SECONDARY == type_) {
            // 如果辅助线程没有获取到的话，还需要再尝试从长时间任务队列中，获取一次
            result = pool_priority_task_queue_->tryPop(task);
        }
        return result;
    }


    /**
     * 从线程池的队列中中，获取批量任务
     * @param tasks
     * @return
     */
    virtual bool popPoolTask(UTaskArrRef tasks) {
        bool result = pool_task_queue_->tryPop(tasks, config_->max_pool_batch_size_);
        if (!result && THREAD_TYPE_SECONDARY == type_) {
            result = pool_priority_task_queue_->tryPop(tasks, 1);    // 从优先队列里，最多pop出来一个
        }
        return result;
    }


    /**
     * 执行单个任务
     * @param task
     */
    CVoid runTask(UTask& task) {
        is_running_ = true;
        task();
        total_task_num_++;
        is_running_ = false;
    }


    /**
     * 批量执行任务
     * @param tasks
     */
    CVoid runTasks(UTaskArr& tasks) {
        is_running_ = true;
        for (auto& task : tasks) {
            task();
        }
        total_task_num_ += tasks.size();
        is_running_ = false;
    }


    /**
     * 清空所有任务内容
     */
    CVoid reset() {
        done_ = false;
        if (thread_.joinable()) {
            thread_.join();    // 等待线程结束
        }
        is_init_ = false;
        is_running_ = false;
        total_task_num_ = 0;
    }


    /**
    * 设置线程优先级，仅针对非windows平台使用
    * 如果设置优先级的话，也就不需要使用优先级队列
    */
    CVoid setSchedParam() {
#ifndef _WIN32
        int priority = THREAD_SCHED_OTHER;
        int policy = THREAD_MIN_PRIORITY;
        if (type_ == THREAD_TYPE_PRIMARY) {
            priority = config_->primary_thread_priority_;
            policy = config_->primary_thread_policy_;
        } else if (type_ == THREAD_TYPE_SECONDARY) {
            priority = config_->secondary_thread_priority_;
            policy = config_->secondary_thread_policy_;
        }

        auto handle = thread_.native_handle();
        sched_param param = { calcPriority(priority) };
        int ret = pthread_setschedparam(handle, calcPolicy(policy), &param);
        if (0 != ret) {
            std::cout << "warning : set thread sched param failed, error code is " <<  ret << std::endl;
        }
#endif
    }

    /**
     * 设置线程亲和性，仅针对linux系统
     */
    CVoid setAffinity(int index) {
#ifdef __linux__
        if (!config_->bind_cpu_enable_ || CPU_NUM == 0 || index < 0) {
            return;
        }

        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(index % CPU_NUM, &mask);

        auto handle = thread_.native_handle();
        int ret = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &mask);
        if (0 != ret) {
            std::cout << "warning : set thread affinity failed, error code is " << ret << std::endl;
        }
#endif
    }


private:
    /**
     * 设定计算线程调度策略信息，
     * 非OTHER/RR/FIFO对应数值，统一返回OTHER类型
     * @param policy
     * @return
     */
    [[nodiscard]] static int calcPolicy(int policy) {
        return (THREAD_SCHED_OTHER == policy
                || THREAD_SCHED_RR == policy
                || THREAD_SCHED_FIFO == policy)
               ? policy : THREAD_SCHED_OTHER;
    }


    /**
     * 设定线程优先级信息
     * 超过[min,max]范围，统一设置为min值
     * @param priority
     * @return
     */
    [[nodiscard]] static int calcPriority(int priority) {
        return (priority >= THREAD_MIN_PRIORITY
                && priority <= THREAD_MAX_PRIORITY)
               ? priority : THREAD_MIN_PRIORITY;
    }


protected:
    bool done_;                                                        // 线程状态标记
    bool is_init_;                                                     // 标记初始化状态
    bool is_running_;                                                  // 是否正在执行
    int type_ = 0;                                                     // 用于区分线程类型（主线程、辅助线程）
    unsigned long total_task_num_ = 0;                                 // 处理的任务的数字

    UAtomicQueue<UTask>* pool_task_queue_;                             // 用于存放线程池中的普通任务
    UAtomicPriorityQueue<UTask>* pool_priority_task_queue_;            // 用于存放线程池中的包含优先级任务的队列，仅辅助线程可以执行
    UThreadPoolConfigPtr config_ = nullptr;                            // 配置参数信息
    std::thread thread_;                                               // 线程类
};


#endif //UTHREADBASE_H
