#pragma once

inline auto ReadFromFile(const std::filesystem::path& filePath, const std::size_t maxBytesToRead) {
  std::ifstream file{ filePath, std::ios::binary };
  if (!file) {
    std::error_code ec(errno, std::generic_category());
    THROW_WEXCEPTION(L"Unable to open file: '{}'. Error message: {}", filePath, ToUtf16(ec.message()));
  }

  std::vector<char> bytes;
  bytes.resize(maxBytesToRead);
  file.read(bytes.data(), bytes.size());
  if (file.bad()) {
    THROW_WEXCEPTION(L"Error reading file: '{}'", filePath);
  }
  bytes.resize(file.gcount());

  return bytes;
}

inline auto ReadFromStream(const LPSTREAM stream, const ULONG maxBytesToRead) {
  std::vector<char> bytes;
  bytes.resize(maxBytesToRead);

  ULONG bytesRead = 0;
  THROW_IF_HR_FAILED_MSG(
    stream->Read(bytes.data(), maxBytesToRead, &bytesRead),
    L"Failed to read {} bytes from stream", maxBytesToRead);

  bytes.resize(bytesRead);
  return bytes;
}
