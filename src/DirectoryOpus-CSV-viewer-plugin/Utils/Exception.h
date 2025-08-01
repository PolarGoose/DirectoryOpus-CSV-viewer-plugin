#pragma once

#define CATCH_ALL_EX \
  catch(const std::exception& e) { \
    DEBUG_LOG(L"Exception: type={} message: {}", ToUtf16(typeid(e).name()), ToUtf16(e.what())); \
  } \
  catch(...) { \
    DEBUG_LOG(L"Unknown exception"); \
  }

#define CATCH_ALL_AND_RETURN(returnValue) \
  catch(const std::exception& e) { \
    DEBUG_LOG(L"Exception: type={} message: {}", ToUtf16(typeid(e).name()), ToUtf16(e.what())); \
    return returnValue; \
  } \
  catch(...) { \
    DEBUG_LOG(L"Unknown exception"); \
    return returnValue; \
  }

#define THROW_WEXCEPTION(...) do { throw WException(std::format(__VA_ARGS__)); } while(0)
#define THROW_WINAPI_EX(winApiFuncName) do { throw WinApiException(BOOST_PP_WSTRINGIZE(winApiFuncName)); } while(0)
#define THROW_WINAPI_EX_MSG(winApiFuncName, ...) do { throw WinApiException(BOOST_PP_WSTRINGIZE(winApiFuncName), std::format(__VA_ARGS__)); } while(0)

class WException : public std::exception
{
public:
  explicit WException(const std::wstring_view msg, const std::source_location& loc = std::source_location::current())
    : _what{
        ToUtf8(std::format(L"{}:{}: {}", ToUtf16(loc.function_name()), loc.line(), msg)) } { }

  const char* what() const noexcept override { return _what.c_str(); }

private:
  std::string _what;
};

class WinApiException final: public WException {
public:
  explicit WinApiException(std::wstring_view winApiFuncName, std::wstring_view message = L"")
    : WException{
        std::format(L"WinApi function '{}' failed. {}. ErrorCode: 0x{:08X}({}). ErrorMessage: {}",
          winApiFuncName, message, GetLastError(), GetLastError(), GetLastErrorMessage()) } { }

private:
  auto GetLastErrorMessage() const {
    return ToUtf16(boost::system::error_code(GetLastError(), boost::system::system_category()).message());
  }
};
