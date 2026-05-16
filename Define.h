#pragma once
#include <string>
#include <wx/wx.h>

// Простые макросы для работы с русским текстом
#ifdef _WIN32
#define RUS(text) EncodingHelper::ToWxString(text).mb_str().data()
#else
#define RUS(text) text
#endif

// Или просто используйте английский везде для надёжности
#define ERROR_MSG(msg) std::string(msg)
