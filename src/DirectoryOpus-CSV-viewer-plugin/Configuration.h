#pragma once

// Plugin configuration. Persisted via the Directory Opus plugin helper.
struct Configuration final {
  enum class DelimiterMode : DWORD {
    Auto = 0,
    Comma = 1,
    Semicolon = 2,
    Tab = 3,
    Pipe = 4,
    Caret = 5,
  };

  enum class HeaderMode : DWORD {
    Auto = 0,
    Yes = 1,
    No = 2,
  };

  static constexpr DWORD DefaultReadSizeMb = 1;
  static constexpr DWORD MaxReadSizeMb = 10;

  DWORD delimiter = static_cast<DWORD>(DelimiterMode::Auto);
  DWORD header = static_cast<DWORD>(HeaderMode::Auto);
  DWORD readSizeMb = DefaultReadSizeMb;

  DelimiterMode GetDelimiterMode() const { return static_cast<DelimiterMode>(delimiter); }
  HeaderMode GetHeaderMode() const { return static_cast<HeaderMode>(header); }

  std::size_t GetReadSizeBytes() const {
    auto mb = readSizeMb == 0 ? DefaultReadSizeMb : readSizeMb;
    if (mb > MaxReadSizeMb) {
      mb = MaxReadSizeMb;
    }
    return static_cast<std::size_t>(mb) * 1024u * 1024u;
  }
};

class ConfigurationStore final : boost::noncopyable {
public:
  inline static ConfigurationStore& GetInstance() {
    static ConfigurationStore instance;
    return instance;
  }

  Configuration GetConfig() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _config;
  }

  bool Save(Configuration config) {
    std::lock_guard<std::mutex> lock(_mutex);
    _config = config;
    std::vector<DOPUSPLUGINCONFIGITEM> items;
    auto configStruct = BuildConfigStruct(items);
    return _helper.LoadOrSaveConfig(OPUSCFG_SAVE, &configStruct) ? true : false;
  }

private:
  ConfigurationStore() {
    std::lock_guard<std::mutex> lock(_mutex);
    std::vector<DOPUSPLUGINCONFIGITEM> items;
    auto configStruct = BuildConfigStruct(items);
    _helper.LoadOrSaveConfig(OPUSCFG_LOAD, &configStruct);
  }

  DOPUSPLUGINCONFIGDATA BuildConfigStruct(std::vector<DOPUSPLUGINCONFIGITEM>& items) {
    items.push_back({ .pszName = L"Delimiter",  .iType = DPCITYPE_DWORD, .pData = &_config.delimiter,  .dwDataSize = sizeof(_config.delimiter)  });
    items.push_back({ .pszName = L"Header",     .iType = DPCITYPE_DWORD, .pData = &_config.header,     .dwDataSize = sizeof(_config.header)     });
    items.push_back({ .pszName = L"ReadSizeMb", .iType = DPCITYPE_DWORD, .pData = &_config.readSizeMb, .dwDataSize = sizeof(_config.readSizeMb) });
    return { .cbSize = sizeof(DOPUSPLUGINCONFIGDATA),
             .pszName = L"DirectoryOpus-CSV-viewer-plugin-configuration",
             .fStateData = FALSE,
             .iNumCfgItems = static_cast<int>(items.size()),
             .pCfgItemArray = items.data(), };
  }

  mutable std::mutex _mutex;
  Configuration _config{};
  DOpusPluginHelperConfig _helper{};
};
