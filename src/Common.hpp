#ifndef CLIENSERVERECN_COMMON_HPP
#define CLIENSERVERECN_COMMON_HPP

#include <string>
#include <string_view>

using namespace std::literals;

static short port = 5555;

struct DealData {
    int dollars = 0;
    int rubles = 0;

    bool IsValid() {
        if (dollars < 0 || rubles < 0) {
            dollars = 0;
            rubles = 0;
            return false;
        }
        return true;
    }
};

struct Requests {
    constexpr static std::string_view REG = "REG"sv;
    constexpr static std::string_view BUY = "BUY"sv;
    constexpr static std::string_view SELL = "SELL"sv;
    constexpr static std::string_view DEAL = "DEAL"sv;
};

struct ResponseCode {
    constexpr static std::string_view OK = "OK"sv;
    constexpr static std::string_view ERROR = "ERROR"sv;
};

// namespace Requests
// {
//     static std::string Registration = "Reg";
//     static std::string Hello = "Hel";
//     static std::string Buy = "Buy";
//     static std::string Sell = "Sell";
//     static std::string Deal = "Deal";
// }

#endif //CLIENSERVERECN_COMMON_HPP
