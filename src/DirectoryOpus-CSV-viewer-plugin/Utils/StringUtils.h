#pragma once

inline std::wstring ToUtf16(const std::string_view utf8Str) {
  return boost::locale::conv::utf_to_utf<wchar_t>(utf8Str.data(), utf8Str.data() + utf8Str.size());
}

inline std::string ToUtf8(std::wstring_view wideStr) {
  return boost::locale::conv::utf_to_utf<char>(wideStr.data(), wideStr.data() + wideStr.size());
}

inline std::u32string ToUtf32(std::string_view wideStr) {
  return boost::locale::conv::utf_to_utf<char32_t>(wideStr.data(), wideStr.data() + wideStr.size());
}

// Allow std::format(L"..") to format std::filesystem::path
namespace std {
  template <>
  struct std::formatter<std::filesystem::path, wchar_t>
    : std::formatter<std::wstring, wchar_t> {
    auto format(const std::filesystem::path& path, wformat_context& ctx) const {
      return formatter<wstring, wchar_t>::format(path.c_str(), ctx);
    }
  };
}
