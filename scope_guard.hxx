#pragma once
#include <algorithm>

template<typename F>
class ScopeGuard
{
public:
    ScopeGuard(F&& func) : _func(std::move(std::forward<F>(func))) { }

    ScopeGuard(ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&& other) = delete;
    ScopeGuard& operator=(ScopeGuard&) = delete;
    ScopeGuard& operator=(ScopeGuard&& other) = delete;

    ~ScopeGuard() {
        _func();
    }

private:
    F _func;
};

struct deferrer {
    template<typename F>
    ScopeGuard<F> operator<<(F&& f) {
        return ScopeGuard<F>(std::forward<F>(f));
    }
};

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define scope_guard auto TOKENPASTE2(__deferred_lambda_call, __COUNTER__) = deferrer() << [&]
