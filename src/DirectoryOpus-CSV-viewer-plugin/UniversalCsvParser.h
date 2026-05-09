#pragma once

// Encapsulates a 3rd party CSV parsing library.
// Automatically detects the separator and whether the CSV has a header row if specified in the config
class UniversalCsvParser final : boost::noncopyable {
public:
  explicit UniversalCsvParser(const std::span<const char> csvFileBytes) {
    auto utf8FileContent = BytesToUtf8(csvFileBytes);
    const auto& [firstLine, secondLine] = GetFirstTwoLines(utf8FileContent);

    const auto& config = ConfigurationStore::GetInstance().GetConfig();
    const auto separator = ResolveSeparator(config.GetDelimiterMode(), firstLine, secondLine);
    _hasHeader = ResolveHasHeader(config.GetHeaderMode(), firstLine, secondLine);

    csv::CSVFormat format;
    format.delimiter(separator);
    if (_hasHeader) {
      format.header_row(0);
    } else {
      format.no_header();
    }

    std::istringstream bufferStream(std::move(utf8FileContent));
    _csvReader.emplace(bufferStream, format);

    _columnCount = _csvReader->get_col_names().size();

    // Materialize all rows for random access
    for (auto& row : *_csvReader) {
      _rows.push_back(std::move(row));
      if (_columnCount == 0 && !_rows.empty()) {
        _columnCount = _rows[0].size();
      }
    }
  }

  auto GetColumnCount() const {
    return _columnCount;
  }

  auto GetColumnName(const std::size_t columnIndex) const {
    if (_hasHeader) {
      const auto& names = _csvReader->get_col_names();
      if (columnIndex < names.size()) {
        return ToUtf16(names[columnIndex]);
      }
    }
    return std::format(L"Col #{}", columnIndex + 1);
  }

  auto GetRowCount() const {
    return _rows.size();
  }

  auto GetRow(const std::size_t rowIndex) const {
    std::vector<std::wstring> result;
    for (const auto& field : _rows[rowIndex]) {
      result.push_back(ToUtf16(field));
    }
    return result;
  }

  auto GetCell(const std::size_t rowIndex, const std::size_t columnIndex) const {
    return ToUtf16(_rows[rowIndex][columnIndex]);
  }

private:
  std::optional<csv::CSVReader> _csvReader;
  std::vector<csv::CSVRow> _rows;
  bool _hasHeader;
  std::size_t _columnCount = 0;

  static std::pair<std::string_view, std::string_view> GetFirstTwoLines(const std::string_view fileContent) {
    auto lines = fileContent | std::views::split('\n');
    auto it = lines.begin();
    if (it == lines.end()) return { {}, {} };

    const std::string_view firstLine(*it);
    if (++it == lines.end()) return { firstLine, {} };

    return { firstLine, std::string_view(*it) };
  }

  static auto CountChar(const std::string_view str, char ch) {
    return std::ranges::count(str, ch);
  }

  static std::size_t CountLetters(const std::string_view str) {
    using Traits = boost::locale::utf::utf_traits<char>;
    std::size_t count = 0;
    for (auto it = str.begin(), end = str.end(); it != end;) {
      const auto codePoint = Traits::decode(it, end);
      if (codePoint != boost::locale::utf::illegal && codePoint != boost::locale::utf::incomplete && std::iswalpha(static_cast<wint_t>(codePoint))) {
        ++count;
      }
    }
    return count;
  }

  static char ResolveSeparator(const Configuration::DelimiterMode mode, const std::string_view firstLine, const std::string_view secondLine) {
    switch (mode) {
    case Configuration::DelimiterMode::Comma:     return ',';
    case Configuration::DelimiterMode::Semicolon: return ';';
    case Configuration::DelimiterMode::Tab:       return '\t';
    case Configuration::DelimiterMode::Pipe:      return '|';
    case Configuration::DelimiterMode::Caret:     return '^';
    case Configuration::DelimiterMode::Auto:      return DetectSeparator(firstLine, secondLine);
    }

    std::unreachable();
  }

  static bool ResolveHasHeader(const Configuration::HeaderMode mode, const std::string_view firstLine, const std::string_view secondLine) {
    switch (mode) {
    case Configuration::HeaderMode::Yes:  return true;
    case Configuration::HeaderMode::No:   return false;
    case Configuration::HeaderMode::Auto: return DetectIfHasHeader(firstLine, secondLine);
    }

    std::unreachable();
  }

  static char DetectSeparator(const std::string_view firstLine, const std::string_view secondLine) {
    const auto count = [&](char sep) { return CountChar(firstLine, sep) + CountChar(secondLine, sep); };

    const std::pair<char /* separator */, std::size_t /* count */> candidates[] = {
        {',',  count(',')},
        {';',  count(';')},
        {'\t', count('\t')},
        {'|',  count('|')},
        {'^',  count('^')}
    };

    return std::ranges::max_element(candidates, {}, [](const auto& p) { return p.second; })->first;
  }

  static bool DetectIfHasHeader(const std::string_view firstLine, const std::string_view secondLine) {
    if (secondLine.empty()) {
      return false;
    }
    return CountLetters(firstLine) > CountLetters(secondLine);
  }
};
