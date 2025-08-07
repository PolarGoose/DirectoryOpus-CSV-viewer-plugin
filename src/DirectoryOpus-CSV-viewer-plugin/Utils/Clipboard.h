#pragma once

inline void CopyTextToClipboard(const HWND control, const std::wstring& text) {
  TRACE_METHOD;

  if (!OpenClipboard(control)) {
    THROW_WINAPI_EX_MSG(OpenClipboard, L"Failed to open the clipboard");
  }

  const boost::scope::scope_exit clipboardCloser([] { CloseClipboard(); });

  if (!EmptyClipboard()) {
    THROW_WINAPI_EX_MSG(EmptyClipboard, L"Failed to clear the clipboard");
  }

  const auto& textNumberOfBytes = (text.size() + 1) * sizeof(wchar_t); // +1 for the null terminator
  const auto& clipboardBuf = static_cast<wchar_t*>(GlobalAlloc(GMEM_FIXED, textNumberOfBytes));
  if (!clipboardBuf) {
    THROW_WINAPI_EX_MSG(GlobalAlloc, L"Failed to allocate memory for the clipboard buffer");
  }

  boost::scope::scope_exit memoryDeallocator([&] { GlobalFree(clipboardBuf); });

  std::memcpy(clipboardBuf, text.c_str(), textNumberOfBytes);
  if (!SetClipboardData(CF_UNICODETEXT, clipboardBuf)) {
    THROW_WINAPI_EX_MSG(SetClipboardData, L"Failed to set the clipboard data");
  };

  memoryDeallocator.set_active(false); // SetClipboardData takes ownership of the memory
}
