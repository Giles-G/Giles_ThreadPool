/***************************
@File: UTask.h
@Desc:
***************************/

#ifndef CGRAPH_UTASK_H
#define CGRAPH_UTASK_H

#include <memory>
#include <type_traits>
#include <vector>
#include "../ThreadPoolinc.hpp"

class UTask {
    struct taskBased {
        explicit taskBased() = default;
        virtual CVoid call() = 0;
        virtual ~taskBased() = default;
    };

    // 退化以获得实际类型，修改思路参考：https://github.com/ChunelFeng/CThreadPool/pull/3
    template <typename F, typename T = typename std::decay<F>::type>
    struct taskDerided : taskBased {
        T func_;
        explicit taskDerided(F&& func) : func_(std::forward<F>(func)) {}
        CVoid call() override { func_(); }
    };

   public:
    template <typename F>
    UTask(F&& f, int priority = 0)
        : impl_(new taskDerided<F>(std::forward<F>(f))), priority_(priority) {}

    CVoid operator()() { impl_->call(); }

    UTask() = default;

    UTask(UTask&& task) noexcept
        : impl_(std::move(task.impl_)), priority_(task.priority_) {}

    UTask& operator=(UTask&& task) noexcept {
        impl_ = std::move(task.impl_);
        priority_ = task.priority_;
        return *this;
    }

    CBool operator>(const UTask& task) const {
        return priority_ < task.priority_;  // 新加入的，放到后面
    }

    CBool operator<(const UTask& task) const {
        return priority_ >= task.priority_;
    }

    NO_ALLOWED_COPY(UTask)

   private:
    std::unique_ptr<taskBased> impl_ = nullptr;
    int priority_ = 0;  // 任务的优先级信息
};

using UTaskRef = UTask&;
using UTaskPtr = UTask*;
using UTaskArr = std::vector<UTask>;
using UTaskArrRef = std::vector<UTask>&;

#endif  // CGRAPH_UTASK_H
