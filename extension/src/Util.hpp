#pragma once
#include <string_view>
#include <vector>

namespace Util {
    // Convert a wide Unicode string to an UTF8 string
    std::string UTF16ToUTF8(std::wstring_view wstr);
    // Convert an UTF8 string to a wide Unicode String
    std::wstring UTF8ToUTF16(std::string_view str);


    inline std::vector<std::string_view> SplitString(std::string_view s, char delim) {
        std::vector<std::string_view> elems;
        std::string::size_type lastPos = 0;
        const std::string::size_type length = s.length();

        while (lastPos < length + 1) {
            std::string::size_type pos = s.find_first_of(delim, lastPos);
            if (pos == std::string::npos) {
                pos = length;
            }

            //if (pos != lastPos || !trimEmpty)
            elems.emplace_back(s.data() + lastPos, pos - lastPos);

            lastPos = pos + 1;
        }

        return elems;
    }


}

#include <vector>
#include <functional>

template<class Sig>
class Signal;

template<typename ReturnType, class... Args>
class Signal<ReturnType(Args...)> {
private:
    typedef std::function<ReturnType(Args...)> Slot;

public:
    void Connect(Slot slot) {
        slots.push_back(slot);
    }

    auto operator() (Args... args) const {
        return Emit(args...);
    }
    auto Emit(Args... args) const {
        if constexpr(std::is_same_v<void, ReturnType>)
        {
            if (slots.empty())
                return;

            for (auto& slot : slots) {
                slot(args...);
            }

            return;
        } else {
            std::vector<ReturnType> returnData;

            if (slots.empty())
                return returnData;

            for (auto& slot : slots) {
                returnData.push_back(slot(args...));
            }

            return returnData;
        }
    }
    void removeAllSlots() {
        slots.clear();
    }
private:
    std::vector<Slot> slots{};
};
