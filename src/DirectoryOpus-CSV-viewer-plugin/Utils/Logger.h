#pragma once

#ifdef DEBUG
  #define PRIVATE_PRINT_TO_DEBUG(...) \
    do { \
      OutputDebugStringW(std::format(L"CsvViewerPlugin(threadId={}):{}", std::this_thread::get_id(), std::format(__VA_ARGS__)).c_str()); \
    } while (0)
#else
  #define PRIVATE_PRINT_TO_DEBUG(...) (void)(__VA_ARGS__)
#endif

#define DEBUG_LOG(...) \
  do { \
    PRIVATE_PRINT_TO_DEBUG(L"{}:{}: {}", ToUtf16(__builtin_FUNCSIG()), __builtin_LINE(), std::format(__VA_ARGS__)); \
  } while (0)
