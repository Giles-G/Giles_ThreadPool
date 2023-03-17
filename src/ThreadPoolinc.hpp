#ifndef THREADPOOL_HPP_
#define THREADPOOL_HPP_

#include <stdlib.h>
#include <functional>
#include <mutex>
#include <memory>
#include <thread>
#include <iostream>
#include "./CStatus.hpp"

#define MAX_SIZE 1024

/** 定义为不能赋值和拷贝的对象类型 */
#define NO_ALLOWED_COPY(CType)                                   \
    CType(const CType &) = delete;                                      \
    const CType &operator=(const CType &) = delete;                     \

using CChar = char;
using CUint = unsigned int;
using CVoid = void;
using CInt = int;
using CLong = long;
using CULong = unsigned long;
using CBool = bool;
using CBigBool = int;
using CFloat = float;
using CDouble = double;
using CConStr = const char*;
using CSize = std::size_t;
using CMSec = int;
using CIndex = int;

using CStatus = CSTATUS;
using CException = CEXCEPTION;

using LOCK_GUARD = std::lock_guard<std::mutex>;
using UNIQUE_LOCK = std::unique_lock<std::mutex>;


static const int CPU_NUM = (int)std::thread::hardware_concurrency(); //CPU数量

/* 线程类型：主线程 or 辅助线程 */
static const int THREAD_TYPE_PRIMARY = 1;
static const int THREAD_TYPE_SECONDARY = 2;

/* 线程调度方法 */
static const int THREAD_SCHED_OTHER = SCHED_OTHER;
static const int THREAD_SCHED_RR = SCHED_RR;
static const int THREAD_SCHED_FIFO = SCHED_FIFO;

static const int THREAD_MIN_PRIORITY = 0;                                            // 线程最低优先级
static const int THREAD_MAX_PRIORITY = 99;                                           // 线程最高优先级
static const CMSec MAX_BLOCK_TTL = 10000000;                                         // 最大阻塞时间，单位为ms
static const CUint DEFAULT_RINGBUFFER_SIZE = 1024;                                   // 默认环形队列的大小
const static CIndex SECONDARY_THREAD_COMMON_ID = -1;                                 // 辅助线程统一id标识

static const int DEFAULT_TASK_STRATEGY = -1;                                         // 默认线程调度策略
static const int LONG_TIME_TASK_STRATEGY = -101;                                     // 长时间任务调度策略
static const int REGION_TASK_STRATEGY = -102;                                        // region的调度策略
static const int EVENT_TASK_STRATEGY = -103;                                         // event的调度策略

/**
 * 以下为线程池配置信息
 */
static const int DEFAULT_THREAD_SIZE = 8;                                            // 默认主线程个数
static const int SECONDARY_THREAD_SIZE = 0;                                          // 默认开启辅助线程个数
static const int MAX_THREAD_SIZE = (DEFAULT_THREAD_SIZE * 2) + 1;                    // 最大线程个数
static const int MAX_TASK_STEAL_RANGE = 2;                                           // 盗取机制相邻范围
static const bool BATCH_TASK_ENABLE = false;                                         // 是否开启批量任务功能
static const int MAX_LOCAL_BATCH_SIZE = 2;                                           // 批量执行本地任务最大值
static const int MAX_POOL_BATCH_SIZE = 2;                                            // 批量执行通用任务最大值
static const int MAX_STEAL_BATCH_SIZE = 2;                                           // 批量盗取任务最大值
static const bool FAIR_LOCK_ENABLE = false;                                          // 是否开启公平锁（非必须场景不建议开启，开启后BATCH_TASK_ENABLE无效）
static const int SECONDARY_THREAD_TTL = 10;                                          // 辅助线程ttl(time to live)，单位为s
static const bool MONITOR_ENABLE = true;                                             // 是否开启监控程序（如果不开启，辅助线程策略将失效。建议开启）
static const int MONITOR_SPAN = 5;                                                   // 监控线程执行间隔，单位为s
static const bool BIND_CPU_ENABLE = true;                                            // 是否开启绑定cpu模式（仅针对主线程）
static const int PRIMARY_THREAD_POLICY = THREAD_SCHED_OTHER;                  // 主线程调度策略
static const int SECONDARY_THREAD_POLICY = THREAD_SCHED_OTHER;                // 辅助线程调度策略
static const int PRIMARY_THREAD_PRIORITY = THREAD_MIN_PRIORITY;               // 主线程调度优先级（取值范围0~99）
static const int SECONDARY_THREAD_PRIORITY = THREAD_MIN_PRIORITY;             // 辅助线程调度优先级（取值范围0~99）

#endif