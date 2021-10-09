#include <Util.hpp>
#include <Windows.h>


std::string Util::UTF16ToUTF8(std::wstring_view wstr)
{
    if (wstr.empty()) return {};
    const int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), &strTo[0], sizeNeeded, nullptr, nullptr);
    return strTo;
}

std::wstring Util::UTF8ToUTF16(std::string_view str)
{
    if (str.empty()) return {};
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), nullptr, 0);
    std::wstring wstrTo(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wstrTo[0], sizeNeeded);
    return wstrTo;
}