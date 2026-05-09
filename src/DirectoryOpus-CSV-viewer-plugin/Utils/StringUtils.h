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

inline auto BytesToUtf8(const std::span<const char> fileStartingBytes) {
  std::string_view rawData{ fileStartingBytes.data(), fileStartingBytes.size() };

  if (rawData.starts_with("\xEF\xBB\xBF")) { // already UTF‑8, just skip BOM
    return std::string(rawData.substr(3));
  }

  if (rawData.starts_with("\xFF\xFE")) { // UTF-16 LE
    std::wstring_view wideRawData{reinterpret_cast<const wchar_t*>(rawData.data() + 2), (rawData.size() - 2) / sizeof(wchar_t)};
    return ToUtf8(wideRawData);
  }

  if (rawData.starts_with("\xFE\xFF")) { // UTF-16 BE
    // Swap bytes to make it little-endian (native wchar_t on Windows)
    auto wbuf = std::span<const char>(rawData.data() + 2, rawData.size() - 2)
      | std::views::chunk(2)
      | std::views::transform([](auto pair) { return static_cast<wchar_t>((static_cast<unsigned char>(pair[0]) << 8) | static_cast<unsigned char>(pair[1])); })
      | std::ranges::to<std::wstring>();
    return ToUtf8(wbuf);
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
