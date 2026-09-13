#include <print>
#include <coroutine>
#include <thread>
#include <cassert>
#include <Windows.h>

struct Task
{
  struct promise_type
  {
    auto get_return_object() noexcept -> Task { return {}; }
    auto initial_suspend() noexcept -> std::suspend_never { return {}; }
    auto final_suspend() noexcept -> std::suspend_never { return {}; }
    void return_void() noexcept {}
    void unhandled_exception() noexcept {}
  };
};

struct Awaitable
{
  auto await_ready() noexcept { return false; }

  void await_suspend(std::coroutine_handle<> h) noexcept
  {
    auto& out = *p_out;
    out = std::jthread([h] { h.resume(); });
    Sleep(3000);
  }

  void await_resume() noexcept {}
  
  std::jthread* p_out;
};

auto test(auto& j) noexcept -> Task
{
  co_await Awaitable{&j};
}

struct Generator
{
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  handle_type _h;

  struct promise_type
  {
    int _v;
    
    auto get_return_object() noexcept -> Generator
    {
      std::println("get return object");
      return { handle_type::from_promise(*this) };
    }
    auto initial_suspend() noexcept -> std::suspend_always { return {}; }
    auto final_suspend() noexcept -> std::suspend_always { return {}; }
    void unhandled_exception() noexcept {}

    auto yield_value(int i) noexcept -> std::suspend_always
    {
      _v = i;
      return {};
    }
    void return_void() noexcept {}
  };

  Generator(handle_type h) noexcept : _h(h) {}
  ~Generator() noexcept { _h.destroy(); }

  explicit operator bool() noexcept
  {
    fill();
    return !_h.done();
  }

  auto operator()() noexcept
  {
    fill();
    _full = false;
    return std::move(_h.promise()._v);
  }

private:
  bool _full{};

  void fill() noexcept
  {
    if (!_full)
    {
      _h();
      _full = true;
    }
  }
};

auto fibonacci_sequence(int n) -> Generator
{
  std::println("run body");
  if (n == 0) co_return;

  if (n > 94) exit(EXIT_FAILURE);

  co_yield 0;

  if (n == 1) co_return;

  co_yield 1;

  if (n == 2) co_return;

  auto a = 0;
  auto b = 1;
  for (auto i = 2; i < n; ++i)
  {
    auto s = a + b;
    co_yield s;
    a = b;
    b = s;
  }
}

auto main() -> int
{
  // std::jthread j;
  // test(j);

  auto gen = fibonacci_sequence(10);
  for (auto i = 0; gen; ++i)
    std::println("{} : {}", i, gen());
}
