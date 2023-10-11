#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

template<typename FnSign>
struct Function
{
    static_assert(!std::is_same_v<FnSign, FnSign>, "not a valid function signature");
};

template<typename Ret, typename... Args>
struct Function<Ret(Args...)>
{
  private:
    struct FuncBase
    {
        virtual Ret call(Args... args) = 0;

        // F exists non-pod type.
        virtual ~FuncBase() = default;
    };

    template<typename F>
    struct FuncImpl : FuncBase
    {
        F m_f;

        FuncImpl(F f) : m_f(std::move(f)) { }

        virtual Ret call(Args... args) override
        {
            return std::invoke(m_f, std::forward<Args>(args)...);
            /// simple implemention
            /// return m_f(std::forward<Args>(args)...);
        }
    };

    std::shared_ptr<FuncBase> m_base;


  public:
    Function() = default;   // make m_base initialize the nullptr

    /// no explicit, so allow the lambda expression covert to the Function
    template<typename F,
             typename = std::enable_if_t<std::is_invocable_r_v<Ret, F &, Args...>>>
    Function(F f) : m_base(std::make_shared<FuncImpl<F>>(std::move(f)))
    { }

    Ret operator()(Args... args) const
    {
        if (!m_base) [[unlikely]]
            throw std::runtime_error("function not intiialized");

        return m_base->call(std::forward<Args>(args)...);
    }
};
