/***************************
@File: CFuncType.h
***************************/

#ifndef CFUNCTYPE_H
#define CFUNCTYPE_H

#include <functional>
#include "./ThreadPoolinc.hpp"


using DEFAULT_FUNCTION = std::function<void()>;
using DEFAULT_CONST_FUNCTION_REF = const std::function<void()>&;
using CSTATUS_FUNCTION = std::function<CStatus()>;
using CSTATUS_CONST_FUNCTION_REF = const std::function<CStatus()>&;
using CALLBACK_FUNCTION = std::function<void(CStatus)>;
using CALLBACK_CONST_FUNCTION_REF = const std::function<void(CStatus)>&;


/**
 * 描述函数类型
 */
enum class CFunctionType {
    INIT = 1,              /** 初始化函数 */
    RUN = 2,               /** 执行函数 */
    DESTROY = 3            /** 释放函数 */
};

/** 开启函数流程 */
#define FUNCTION_BEGIN                                           \
    CStatus status;                                                     \

/** 结束函数流程 */
#define FUNCTION_END                                             \
    return status;                                                      \

/** 无任何功能函数 */
#define EMPTY_FUNCTION                                           \
    return CStatus();                                                   \

/** 不支持当前功能 */
#define NO_SUPPORT                                               \
    return CStatus(FUNCTION_NO_SUPPORT);                         \

/** 返回异常信息和状态 */
#define RETURN_ERROR_STATUS(info)                                \
    return CStatus(info);                                               \

/** 定义为不能赋值和拷贝的对象类型 */
#define NO_ALLOWED_COPY(CType)                                   \
    CType(const CType &) = delete;                                      \
    const CType &operator=(const CType &) = delete;                     \

/** 抛出异常 */
#define THROW_EXCEPTION(info)                                    \
    throw CException(info);                                             \

#endif //CFUNCTYPE_H
