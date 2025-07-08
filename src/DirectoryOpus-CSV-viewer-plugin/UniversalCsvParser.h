#pragma once

// Encapsulates a 3rd party CSV parsing library.
// Automatically detects the separator and whether the CSV has a header row.
class UniversalCsvParser final : boost::noncopyable {
public:
  explicit UniversalCsvParser(const std::filesystem::path& filePath) {
    const auto& buffer = ReadFirstBytesFromFile(filePath, 1 * 1024 * 1024 /* 1 MB */);
    const auto& [firstLine, secondLine] = GetFirstTwoLines(std::string_view{ buffer.data(), buffer.size() });
    const auto& separator = DetectSeparator(firstLine, secondLine);
    _hasHeader = DetectIfHasHeader(firstLine, secondLine);

    std::istringstream bufferStream(buffer);
    _csvDoc.emplace(bufferStream, rapidcsv::LabelParams(_hasHeader ? 0: -1, -1), rapidcsv::SeparatorParams(separator));
  }

  auto GetColumnCount() const {
    return _csvDoc->GetColumnCount();
  }

  auto GetColumnName(int columnIndex) const {
    if (_hasHeader) {
      return ToUtf16(_csvDoc->GetColumnName(columnIndex));
    }
    return std::format(L"Col #{}", columnIndex + 1);
  }

  auto GetRowCount() const {
    return _csvDoc->GetRowCount();
  }

  auto GetRow(int rowIndex) const {
    return _csvDoc->GetRow<std::string>(rowIndex)
         | std::views::transform([](const auto& s) { return ToUtf16(s); })
         | std::ranges::to<std::vector<std::wstring>>();
  }

  auto GetCell(int rowIndex, int columnIndex) {
    return ToUtf16(_csvDoc->GetCell<std::string>(columnIndex, rowIndex));
  }

private:
  std::optional<rapidcsv::Document> _csvDoc;
  std::vector<std::wstring> _columnNames;
  bool _hasHeader;

  static std::string ReadFirstBytesFromFile(const std::filesystem::path& filePath, const std::size_t bytesToRead) {
    std::ifstream file{ filePath, std::ios::binary };
    if (!file) {
      std::error_code ec(errno, std::generic_category());
      THROW_WEXCEPTION(L"Unable to open file: '{}'. Error message: {}", filePath, ToUtf16(ec.message()));
    }

    std::string rawData;
    rawData.resize(bytesToRead);
    file.read(rawData.data(), rawData.size());
    if (file.bad()) {
      THROW_WEXCEPTION(L"Error reading file: '{}'", filePath);
    }
    rawData.resize(file.gcount());

    if (rawData.starts_with("\xEF\xBB\xBF")) { // already UTF‑8, just skip BOM
      return rawData.substr(3);
    }

    if (rawData.starts_with("\xFF\xFE")) { // UTF‑16 LE
      return boost::locale::conv::between(rawData, "UTF-8", "UTF-16LE");
    }

    if (rawData.starts_with("\xFE\xFF")) { // UTF‑16 BE
      return boost::locale::conv::between(rawData, "UTF-8", "UTF-16BE");
    }

    return rawData; // assume UTF‑8 no BOM
  }

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
    const auto& commas = CountChar(firstLine, ',') + CountChar(secondLine, ',');
    const auto& semis = CountChar(firstLine, ';') + CountChar(secondLine, ';');
    return semis > commas ? ';' : ',';
  }

  static bool DetectIfHasHeader(const std::string_view& firstLine, const std::string_view& secondLine) {
    return CountLetters(firstLine) > CountLetters(secondLine);
  }
};
