#pragma once
#include <string_view>

namespace Util {
    // Convert a wide Unicode string to an UTF8 string
    std::string UTF16ToUTF8(std::wstring_view wstr);
    // Convert an UTF8 string to a wide Unicode String
    std::wstring UTF8ToUTF16(std::string_view str);
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