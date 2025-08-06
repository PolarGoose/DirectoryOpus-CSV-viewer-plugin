#pragma once

class CsvViewerControl final: public CWindowImpl<CsvViewerControl, CListViewCtrl>, boost::noncopyable {
public:
  DECLARE_WND_SUPERCLASS(L"DirectoryOpus.PolarGoose.CsvViewerControl", WC_LISTVIEWW)

  BEGIN_MSG_MAP(CsvViewerControl)
    MESSAGE_HANDLER(DVPLUGINMSG_GETCAPABILITIES, OnGetCapabilities)
    MESSAGE_HANDLER(DVPLUGINMSG_LOAD, OnLoad)
    MESSAGE_HANDLER(DVPLUGINMSG_LOADSTREAM, OnLoadStream)
  END_MSG_MAP()

  LRESULT OnGetCapabilities(const UINT /* messageType */, const WPARAM /* wParam */, const LPARAM /* lParam */, BOOL& /* handled */) {
    TRACE_METHOD;

    return VPCAPABILITY_WANTMOUSEWHEEL
         | VPCAPABILITY_WANTFOCUS
         | VPCAPABILITY_CANTRACKFOCUS;
  }

  LRESULT OnLoad(const UINT /* messageType */, const WPARAM /* wParam */, const LPARAM lParam, BOOL& /* handled */) try {
    const auto& filePath = reinterpret_cast<LPTSTR>(lParam);
    TRACE_METHOD_MSG(L"filePath={}", filePath);

    const auto& fileBytes = ReadFromFile(filePath, 1 * 1024 * 1024 /* 1 MB */);
    FillListControl(fileBytes);
    return TRUE;
  } CATCH_ALL_AND_RETURN(FALSE)

  LRESULT OnLoadStream(const UINT /* messageType */, const WPARAM wParam, const LPARAM lParam, BOOL& /* handled */) try {
    const auto& filePath = reinterpret_cast<LPTSTR>(wParam);
    TRACE_METHOD_MSG(L"filePath={}", filePath);

    const auto& streamHandle = reinterpret_cast<LPSTREAM>(lParam);
    const auto& fileBytes = ReadFromStream(streamHandle, 1 * 1024 * 1024 /* 1 MB */);
    FillListControl(fileBytes);
    return TRUE;
  } CATCH_ALL_AND_RETURN(FALSE)

  void OnFinalMessage(HWND) override {
    TRACE_METHOD;
    delete this;
  }

  void FillListControl(const std::span<const char> csvFileBytes) {
    // Reset the control to a clean state
    DeleteAllItems();
    for (auto i = GetHeader().GetItemCount() - 1; i >= 0; --i) {
      DeleteColumn(i);
    }

    const UniversalCsvParser csvParser(csvFileBytes);

    // Populate column names
    for (int colIndex = 0; colIndex < csvParser.GetColumnCount(); colIndex++) {
      const auto& colName = csvParser.GetColumnName(colIndex);
      AddColumn(colName.c_str(), colIndex);
    }
    // Populate rows
    for (auto rowIndex = 0; rowIndex < csvParser.GetRowCount(); rowIndex++) {
      const auto& row = csvParser.GetRow(rowIndex);

      const auto& idx = InsertItem(rowIndex, row[0].c_str());
      for (auto colIndex = 1; colIndex < row.size(); colIndex++) {
        SetItemText(idx, colIndex, row[colIndex].c_str());
      }
    }

    // Resize columns to fit content (select the maximum width of the column based on both the content and the header)
    for (int col = 0; col < csvParser.GetColumnCount(); ++col) {
      ListView_SetColumnWidth(m_hWnd, col, LVSCW_AUTOSIZE);
      const auto& columnWithElementBased = ListView_GetColumnWidth(m_hWnd, col);

      ListView_SetColumnWidth(m_hWnd, col, LVSCW_AUTOSIZE_USEHEADER);
      const auto& columnWidthHeaderBased = ListView_GetColumnWidth(m_hWnd, col);

      ListView_SetColumnWidth(m_hWnd, col, std::max(columnWithElementBased, columnWidthHeaderBased));
    }
  }
};

inline HWND CreateCsvViewerCtrl(const HWND parentControlHandle, const LPRECT drawingArea) {
  TRACE_METHOD;

  auto control = std::make_unique<CsvViewerControl>();
  const auto& controlHandle = control->Create(
    /* hWndParent   */ parentControlHandle,
    /* rect         */ drawingArea,
    /* szWindowName */ NULL,
    /* dwStyle      */ WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_SHOWSELALWAYS);

  if (!controlHandle) {
    THROW_WINAPI_EX_MSG(CsvViewerControl::Create, L"Failed to create CsvViewerControl control");
  }

  control->SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

  // CsvViewerControl deletes itself in OnFinalMessage.
  control.release();

  return controlHandle;
}
