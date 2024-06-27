#pragma once
#include <bit>
#include <string_view>
#include <vector>
#include <functional>
#include <cctype>
#include <ranges>

namespace Util
{
    // Convert a wide Unicode string to an UTF8 string
    std::string UTF16ToUTF8(std::wstring_view wstr);
    // Convert an UTF8 string to a wide Unicode String
    std::wstring UTF8ToUTF16(std::string_view str);


    inline std::vector<std::string_view> SplitString(std::string_view s, char delim)
    {
        std::vector<std::string_view> elems;
        std::string::size_type lastPos = 0;
        const std::string::size_type length = s.length();

        while (lastPos < length + 1)
        {
            std::string::size_type pos = s.find_first_of(delim, lastPos);
            if (pos == std::string::npos)
            {
                pos = length;
            }

            //if (pos != lastPos || !trimEmpty)
            elems.emplace_back(s.data() + lastPos, pos - lastPos);

            lastPos = pos + 1;
        }

        return elems;
    }

    inline std::string_view UnQuote(std::string_view input)
    {
        return input.substr(1, input.length() - 2); // "\"test\"" -> "test"
    }

    inline auto SplitHeaderString(std::string_view headers)
    {
        // Split by ;
        // Split each by :
        // Filter out pairs, everything thats not a pair is invalid either too many or too few elements
        return
            Util::SplitString(Util::UnQuote(headers), ';')
            | std::views::transform([](const auto& header)
                {
                    return Util::SplitString(header, ':');
                })
            | std::views::filter([](const auto& headerSplit)
                {
                    return headerSplit.size() == 2;
                })
            | std::views::transform([](const auto& headerSplit)
                {
                    return std::pair<std::string_view, std::string_view>(headerSplit[0], headerSplit[1]);
                });
    }

    // https://github.com/arma3/sqf-value/blob/master/sqf-value/value.hpp#L306
    static std::string ParseSQFString(std::string_view data)
    {
        auto begin = data.begin();
        auto end = data.end();
        // start-char
        char c = *begin;

        // find end
        std::string_view::const_iterator copy;
        size_t quotes = 0;
        for (copy = begin + 1; copy != end; ++copy)
        {
            if (*copy == c)
            {
                ++copy;
                if (copy != end && *copy == c)
                    quotes++;
                else
                    break;
            }
        }
        // create string
        auto len = copy - begin - 2;
        std::string target;
        target.reserve(len - quotes);
        for (++begin; begin != end; ++begin)
        {
            char cur = *begin;
            if (*begin == c)
            {
                ++begin;
                if (begin != end && *begin == c)
                    target.append(&cur, &cur + 1);
                else
                    break;
            }
            else
                target.append(&cur, &cur + 1);
        }
        return target;
    }
}



template <class Sig>
class Signal;

template <typename ReturnType, class... Args>
class Signal<ReturnType(Args ...)>
{
private:
    using Slot = std::function<ReturnType(Args ...)>;

public:
    void Connect(Slot slot)
    {
        slots.push_back(slot);
    }

    auto operator()(Args ... args) const
    {
        return Emit(args...);
    }

    auto Emit(Args ... args) const
    {
        if constexpr (std::is_same_v<void, ReturnType>)
        {
            if (slots.empty())
                return;

            for (auto& slot : slots)
            {
                slot(args...);
            }

            return;
        }
        else
        {
            std::vector<ReturnType> returnData;

            if (slots.empty())
                return returnData;

            for (auto& slot : slots)
            {
                returnData.push_back(slot(args...));
            }

            return returnData;
        }
    }

    void removeAllSlots()
    {
        slots.clear();
    }

private:
    std::vector<Slot> slots{};
};



class FnvHash
{
    static constexpr uint64_t FNV_PRIME = 0x100000001b3;
    static constexpr uint64_t OFFSET_BASIS = 0xcbf29ce484222325;

    uint64_t hashValue = FNV_PRIME;
public:
    constexpr FnvHash() {}

    template <unsigned int N>
    constexpr FnvHash(const char(&str)[N]) { AddString(std::string_view(str, N)); }

    constexpr operator uint64_t() const { return hashValue; }
    constexpr uint64_t GetValue() const { return hashValue; }
    consteval uint64_t GetValueCompiletime() const { return hashValue; }

    constexpr FnvHash& AddString(std::string_view str)
    {
        for (const auto value : str)
        {
            hashValue = hashValue ^ value;
            hashValue *= FNV_PRIME;
        }

        return *this;
    }

    constexpr FnvHash& AddString(std::wstring_view str)
    {
        for (const auto value : str)
        {
            hashValue = hashValue ^ value;
            hashValue *= FNV_PRIME;
        }

        return *this;
    }

    FnvHash& AddStringCI(std::string_view str)
    {
        for (const auto value : str)
        {
            hashValue = hashValue ^ std::tolower(value);
            hashValue *= FNV_PRIME;
        }
    
        return *this;
    }
    
    //FnvHash& AddStringCI(std::wstring_view str)
    //{
    //    for (const auto value : str)
    //    {
    //        hashValue = hashValue ^ std::towlower(value);
    //        hashValue *= FNV_PRIME;
    //    }
    //
    //    return *this;
    //}

    template<std::integral T>
    constexpr FnvHash& Add(T d)
    {
        auto asArray = std::bit_cast<std::array<uint8_t, sizeof(T)>>(d);

        for (const auto value : asArray)
        {
            hashValue = hashValue ^ value;
            hashValue *= FNV_PRIME;
        }

        return *this;
    }
};

consteval uint64_t operator""_fnvHash(const char* str, std::size_t len)
{
    return FnvHash{}.AddString(std::string_view(str, len)).GetValueCompiletime();
}