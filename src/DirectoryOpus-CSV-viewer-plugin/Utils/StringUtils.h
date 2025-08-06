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

inline auto ToUtf8(const std::span<const char> fileStartingBytes) {
  std::string_view rawData{ fileStartingBytes.data(), fileStartingBytes.size() };

  if (rawData.starts_with("\xEF\xBB\xBF")) { // already UTF‑8, just skip BOM
    return std::string(rawData.substr(3));
  }

  if (rawData.starts_with("\xFF\xFE")) { // UTF‑16 LE
    return boost::locale::conv::between(rawData.data() + 2, rawData.data() + rawData.size(), "UTF-8", "UTF-16LE");
  }

  if (rawData.starts_with("\xFE\xFF")) { // UTF‑16 BE
    return boost::locale::conv::between(rawData.data() + 2, rawData.data() + rawData.size(), "UTF-8", "UTF-16BE");
  }

  return std::string(rawData); // assume UTF‑8 no BOM
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
