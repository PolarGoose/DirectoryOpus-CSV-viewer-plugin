#pragma once

// WinApi
#include <windows.h>
#include <winscard.h>

// std
#include <source_location>
#include <exception>
#include <thread>
#include <sstream>
#include <typeinfo>
#include <format>
#include <fstream>
#include <filesystem>
#include <ranges>
#include <algorithm>

// boost
#include <boost/noncopyable.hpp>
#include <boost/locale.hpp>
#include <boost/system.hpp>
#include <boost/preprocessor.hpp>
#include <boost/scope/scope_exit.hpp>

// ICU
#include <unicode/uchar.h>

// rapidcsv
#include <rapidcsv.h>

// WTL
#include <atlbase.h>
#include <atlstr.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlctrls.h>
#include <atlcrack.h>
#include <comdef.h>

// Directory Opus Plugin SDK
#include <DirectoryOpus-plugin-SDK/viewer plugins.h>
#include <DirectoryOpus-plugin-SDK/plugin support.h>
