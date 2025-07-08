#pragma once

#define TRACE_METHOD const MethodTracer::Detail::MethodTracer methodTracerUniquName;
#define TRACE_METHOD_MSG(...) const MethodTracer::Detail::MethodTracer methodTracerUniquName(std::format(__VA_ARGS__));

namespace MethodTracer::Detail {

class MethodTracer final: boost::noncopyable {
public:
  MethodTracer(const std::wstring_view msg = L"", const char* const funcName = __builtin_FUNCSIG()) noexcept
    : funcName(ToUtf16(funcName))
    , startingNumberOfUncaughtExceptions(std::uncaught_exceptions()) {
    PRIVATE_PRINT_TO_DEBUG(L"{} -> {}", this->funcName, msg);
  }

  ~MethodTracer() {
    if (std::uncaught_exceptions() > startingNumberOfUncaughtExceptions) {
      PRIVATE_PRINT_TO_DEBUG(L"{} <- exception", funcName);
    }
    else {
      PRIVATE_PRINT_TO_DEBUG(L"{} <-", funcName);
    }
  }

private:
  const std::wstring funcName;
  const int startingNumberOfUncaughtExceptions;
};

}
