#pragma once

#include "DirectoryOpus-CSV-viewer-plugin/resource.h"
#include "DirectoryOpus-CSV-viewer-plugin/Configuration.h"

class ConfigurationDialog final : public CDialogImpl<ConfigurationDialog>, boost::noncopyable {
public:
  enum { IDD = IDD_CONFIG };
  ConfigurationDialog(const HWND notifyWindow, const DWORD notifyData) : _notifyWindow(notifyWindow), _notifyData(notifyData) { }

  BEGIN_MSG_MAP(ConfigurationDialog)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_NCDESTROY, OnNcDestroy)
    COMMAND_ID_HANDLER(IDOK, OnOk)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
  END_MSG_MAP()

private:
  void OnFinalMessage(HWND /* hWnd */) override {
    delete this;
  }

  LRESULT OnNcDestroy(const UINT /* messageType */, const WPARAM /* wParam */, const LPARAM /* lParam */, BOOL& handled) {
    TRACE_METHOD;
    ::PostQuitMessage(0);
    handled = FALSE;
    return 0;
  }

  static constexpr LPCWSTR DelimiterItems[] = { L"auto", L",", L";", L"\\t", L"|", L"^" };
  static constexpr LPCWSTR HeaderItems[]    = { L"auto", L"yes", L"no" };

  LRESULT OnInitDialog(const UINT /* messageType */, const WPARAM /* wParam */, const LPARAM /* lParam */, BOOL& /* handled */) try {
    TRACE_METHOD;

    const auto& cfg = ConfigurationStore::GetInstance().GetConfig();

    CComboBox delimiterCombo(GetDlgItem(IDC_COMBO_DELIMITER));
    for (const auto& item : DelimiterItems) {
      delimiterCombo.AddString(item);
    }
    delimiterCombo.SetCurSel(static_cast<int>(cfg.delimiter));

    CComboBox headerCombo(GetDlgItem(IDC_COMBO_HEADER));
    for (const auto& item : HeaderItems) {
      headerCombo.AddString(item);
    }
    headerCombo.SetCurSel(static_cast<int>(cfg.header));

    SetDlgItemInt(IDC_EDIT_READSIZE, cfg.readSizeMb, FALSE);

    CenterWindow(GetParent());
    return TRUE;
  } CATCH_ALL_AND_RETURN(FALSE)

  LRESULT OnOk(const WORD /* notifyCode */, const WORD /* id */, const HWND /* hWndCtl */, BOOL& /* handled */) try {
    TRACE_METHOD;

    Configuration cfg;

    const auto delimiterSel = SendDlgItemMessage(IDC_COMBO_DELIMITER, CB_GETCURSEL, 0, 0);
    cfg.delimiter = static_cast<DWORD>(delimiterSel);

    const auto headerSel = SendDlgItemMessage(IDC_COMBO_HEADER, CB_GETCURSEL, 0, 0);
    cfg.header = static_cast<DWORD>(headerSel);

    BOOL ok = FALSE;
    const UINT readSize = GetDlgItemInt(IDC_EDIT_READSIZE, &ok, FALSE);
    cfg.readSizeMb = (ok && readSize > 0) ? readSize : Configuration::DefaultReadSizeMb;
    if (cfg.readSizeMb > Configuration::MaxReadSizeMb) {
      cfg.readSizeMb = Configuration::MaxReadSizeMb;
    }

    ConfigurationStore::GetInstance().Save(cfg);

    ::PostMessage(_notifyWindow, DVPLUGINMSG_REINITIALIZE, 0, _notifyData);

    DestroyWindow();
    return 0;
  } CATCH_ALL_AND_RETURN(0)

  LRESULT OnCancel(const WORD /* notifyCode */, const WORD /* id */, const HWND /* hWndCtl */, BOOL& /* handled */) {
    TRACE_METHOD;
    DestroyWindow();
    return 0;
  }

  HWND _notifyWindow;
  DWORD _notifyData;
};

inline HWND CreateConfigurationDialog(const HWND parentWindow, const HWND notifyWindow, const DWORD notifyData) {
  TRACE_METHOD;

  auto dialog = std::make_unique<ConfigurationDialog>(notifyWindow, notifyData);
  const HWND handle = dialog->Create(parentWindow);
  if (!handle) {
    THROW_WINAPI_EX_MSG(ConfigurationDialog::Create, L"Failed to create the configuration dialog");
  }

  dialog->ShowWindow(SW_SHOW);

  // ConfigurationDialog deletes itself in OnFinalMessage.
  dialog.release();
  return handle;
}
