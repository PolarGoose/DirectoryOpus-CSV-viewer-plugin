#pragma once

// Encapsulates a 3rd party CSV parsing library.
// Automatically detects the separator and whether the CSV has a header row.
class UniversalCsvParser final : boost::noncopyable {
public:
  explicit UniversalCsvParser(const std::span<const char> csvFileBytes) {
    const auto& utf8FileContent = ToUtf8(csvFileBytes);
    const auto& [firstLine, secondLine] = GetFirstTwoLines(utf8FileContent);
    const auto& separator = DetectSeparator(firstLine, secondLine);
    _hasHeader = DetectIfHasHeader(firstLine, secondLine);

    std::istringstream bufferStream(utf8FileContent);
    _csvDoc.emplace(bufferStream, rapidcsv::LabelParams(_hasHeader ? 0 : -1, -1), rapidcsv::SeparatorParams(separator));
  }

  auto GetColumnCount() const {
    return _csvDoc->GetColumnCount();
  }

  auto GetColumnName(const int columnIndex) const {
    if (_hasHeader) {
      return ToUtf16(_csvDoc->GetColumnName(columnIndex));
    }
    return std::format(L"Col #{}", columnIndex + 1);
  }

  auto GetRowCount() const {
    return _csvDoc->GetRowCount();
  }

  auto GetRow(const int rowIndex) const {
    return _csvDoc->GetRow<std::string>(rowIndex)
         | std::views::transform([](const auto& s) { return ToUtf16(s); })
         | std::ranges::to<std::vector<std::wstring>>();
  }

  auto GetCell(const int rowIndex, const int columnIndex) {
    return ToUtf16(_csvDoc->GetCell<std::string>(columnIndex, rowIndex));
  }

private:
  std::optional<rapidcsv::Document> _csvDoc;
  bool _hasHeader;

  static std::pair<std::string_view, std::string_view> GetFirstTwoLines(const std::string_view& fileContent) {
    const auto& firstLineEnd = fileContent.find('\n');
    const auto& firstLine = (firstLineEnd == std::string_view::npos) ? fileContent : fileContent.substr(0, firstLineEnd);

    const auto& secondLineStart = (firstLineEnd == std::string_view::npos) ? fileContent.size() : firstLineEnd + 1;
    if (secondLineStart >= fileContent.size()) {
      return{ firstLine, std::string_view{} };
    }

    const auto& secondLineEnd = fileContent.find('\n', secondLineStart);
    const auto& secondLine = (secondLineEnd == std::string_view::npos)
      ? fileContent.substr(secondLineStart)
      : fileContent.substr(secondLineStart, secondLineEnd - secondLineStart);

    return { firstLine, secondLine };
  }

  static auto CountChar(const std::string_view& str, char ch) {
    return std::ranges::count(str, ch);
  }

  static auto CountLetters(const std::string_view& str) {
    const auto& utf32str = ToUtf32(str);
    return std::ranges::count_if(utf32str, [](const auto& c) { return u_isalpha(c); });
  }

  static char DetectSeparator(const std::string_view& firstLine, const std::string_view& secondLine) {
    const auto count = [&](char sep) { return CountChar(firstLine, sep) + CountChar(secondLine, sep); };

    const std::pair<char /* separator */, std::size_t /* count */> candidates[] = {
        {',',  count(',')},
        {';',  count(';')},
        {'\t', count('\t')},
        {'|',  count('|')}
    };

    return std::ranges::max_element(candidates, {}, [](const auto& p) { return p.second; })->first;
  }

  static bool DetectIfHasHeader(const std::string_view& firstLine, const std::string_view& secondLine) {
    if (secondLine.empty()) {
      return false;
    }

    return CountLetters(firstLine) > CountLetters(secondLine);
  }
};
