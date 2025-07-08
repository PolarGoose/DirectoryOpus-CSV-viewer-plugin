#include "DirectoryOpus-CSV-viewer-plugin/Utils/StringUtils.h"
#include "DirectoryOpus-CSV-viewer-plugin/Utils/Logger.h"
#include "DirectoryOpus-CSV-viewer-plugin/Utils/MethodTracer.h"
#include "DirectoryOpus-CSV-viewer-plugin/Utils/Exception.h"
#include "DirectoryOpus-CSV-viewer-plugin/UniversalCsvParser.h"
#include "DirectoryOpus-CSV-viewer-plugin/CsvViewerControl.h"

#define DLL_EXPORT extern "C" __declspec(dllexport)

// DOpus can call Inti/Uninit multiple times.
// (e.g. Click the Refresh button in the plugins list to trigger an additional Init/Uninit pair.)
static int initRefCount = 0;

extern "C" BOOL WINAPI DllMain(const HINSTANCE /* thisDllModule */, const DWORD reason, const LPVOID /*reserved*/) try {
  if (reason == DLL_PROCESS_ATTACH) {
    DEBUG_LOG(L"DLL_PROCESS_ATTACH");
  }

  if(reason == DLL_PROCESS_DETACH) {
    DEBUG_LOG(L"DLL_PROCESS_DETACH");
  }

  return TRUE;
} CATCH_ALL_AND_RETURN(FALSE)

DLL_EXPORT BOOL DVP_InitEx(const LPDVPINITEXDATA /* pInitExData */) try {
  TRACE_METHOD;

  initRefCount++;

  if(initRefCount > 1) {
    DEBUG_LOG(L"Already initialized. initRefCount={}", initRefCount);
    return TRUE;
  }

  // Nothing needs to be done during plugin initialization
  return TRUE;
} CATCH_ALL_AND_RETURN(FALSE)

DLL_EXPORT void DVP_Uninit(void) try {
  TRACE_METHOD;

  initRefCount--;

  if(initRefCount > 0) {
    DEBUG_LOG(L"No need to uninit. initRefCount={}", initRefCount);
    return;
  }

  // Nothing needs to be done during plugin uninitialization
} CATCH_ALL_EX

DLL_EXPORT BOOL DVP_Identify(const LPVIEWERPLUGININFO info) try {
  TRACE_METHOD;

  info->dwFlags = DVPFIF_ExtensionsOnly
                | DVPFIF_NoThumbnails
                | DVPFIF_NoFileInformation;

  info->lpszHandleExts = const_cast<wchar_t*>(L".csv");
  info->dwlMinFileSize = 1;
  info->dwlMaxFileSize = 0 /* infinite */;
  info->uiMajorFileType = DVPMajorType_Text;
  info->idPlugin = { 0x719a1b08, 0x371a, 0x46cb, { 0x8a, 0x4b, 0xd3, 0x22, 0x5, 0xa1, 0x94, 0x55 } }; // {719A1B08-371A-46CB-8A4B-D32205A19455}
  info->lpszName = const_cast<wchar_t*>(L"PolarGoose CSV viewer");
  info->lpszDescription = const_cast<wchar_t*>(L"CVS viewer plugin for Directory Opus");
  info->lpszURL = const_cast<wchar_t*>(L"https://github.com/PolarGoose/DirectoryOpus-CSV-viewer-plugin");

  // The version format: MAJOR.MINOR.BUILD.REVISION
  info->dwVersionHigh = MAKELONG(/* MINOR */ VERSION_MINOR, /* MAJOR */ VERSION_MAJOR);
  info->dwVersionLow = MAKELONG(/* REVISION */ 0, /* BUILD */ 0);

  return TRUE;
} CATCH_ALL_AND_RETURN(FALSE)

DLL_EXPORT BOOL DVP_IdentifyFile(const HWND  /* parentControlHandle */, const LPTSTR filePath, const LPVIEWERPLUGINFILEINFO fileInfo, const HANDLE /* abortEvent */) try {
  TRACE_METHOD_MSG(L"filePath={}", filePath);

  // We only identify the file by its extension. It is already done my the DOpus.
  // Thus we don't need to do anything.

  fileInfo->dwFlags = DVPFIF_CanReturnViewer;

  return TRUE;
} CATCH_ALL_AND_RETURN(FALSE)

DLL_EXPORT HWND DVP_CreateViewer(const HWND parentControlHandle, const LPRECT drawingArea, DWORD /* drawingFlags */) try {
  TRACE_METHOD;
  return CreateCsvViewerCtrl(parentControlHandle, drawingArea);
} CATCH_ALL_AND_RETURN(nullptr)

DLL_EXPORT BOOL DVP_USBSafe(const LPOPUSUSBSAFEDATA /* uSBSafeData */) {
  TRACE_METHOD;
  return TRUE;
}
