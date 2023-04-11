/***************************
@File: UAllocator.h
***************************/

#ifndef UALLOCATOR_H
#define UALLOCATOR_H

#include <memory>
#include <mutex>

#include "./ThreadPoolinc.hpp"
#include "./CFuncType.hpp"
#include "./CStdEx.hpp"


class CObject {
public:
    /**
     * 默认构造函数
     */
    explicit CObject() = default;

    /**
     * 初始化函数
     */
    virtual CStatus init() {
        EMPTY_FUNCTION
    }

    /**
     * 流程处理函数
     */
    virtual CStatus run() = 0;

    /**
     * 释放函数
     */
    virtual CStatus destroy() {
        EMPTY_FUNCTION
    }

    /**
     * 默认析构函数
     */
    virtual ~CObject() = default;
};

static std::mutex g_session_mtx;

/**
 * 仅用于生成CObject类型的类
 */
class UAllocator : public CObject {
   public:
    /**
     * 生成普通指针信息
     * @tparam T
     * @return
     */
    template <typename T,
              c_enable_if_t<std::is_base_of<CObject, T>::value, int> = 0>
    static T* safeMallocCObject() {
        T* ptr = nullptr;
        while (!ptr) {
            ptr = new (std::nothrow) T();
        }
        return ptr;
    }

    /**
     * 生成带参数的普通指针
     * @tparam T
     * @tparam Args
     * @param args
     * @return
     */
    template <typename T,
              typename... Args,
              c_enable_if_t<std::is_base_of<CObject, T>::value, int> = 0>
    static T* safeMallocTemplateCObject(Args... args) {
        T* ptr = nullptr;
        while (!ptr) {
            ptr = new T(std::forward<Args>(args)...);
        }
        return ptr;
    }

    /**
     * 生成unique智能指针信息
     * @tparam T
     * @return
     */
    template <typename T,
              c_enable_if_t<std::is_base_of<CObject, T>::value, int> = 0>
    static std::unique_ptr<T> makeUniqueCObject() {
        return c_make_unique<T>();
    }
};

#define SAFE_MALLOC_COBJECT(Type) UAllocator::safeMallocCObject<Type>();

#define MAKE_UNIQUE_COBJECT(Type) UAllocator::makeUniqueCObject<Type>();
#endif  // UALLOCATOR_H
