#include <print>
#include <coroutine>

struct Promise;
struct CoroutineHandle : std::coroutine_handle<Promise>
{
  using promise_type = Promise;
};

struct Promise
{
  auto get_return_object() noexcept -> CoroutineHandle
  {
    std::println("get return object");
    return { CoroutineHandle::from_promise(*this) };
  }

  auto initial_suspend() noexcept
  {
    std::println("init suspend");
    return std::suspend_always();
  }

  auto final_suspend() noexcept
  {
    std::println("final suspend");
    return std::suspend_always();
  }

  void return_void() noexcept {}
  void unhandled_exception() noexcept {}
};

struct S
{
  int i;

  auto f() noexcept -> CoroutineHandle
  {
    std::println("call body : {}", i);
    co_return;
  }
};

auto bad() noexcept
{
  S s{0};
  return s.f();
}

auto main() -> int
{
  bad().resume();
}
