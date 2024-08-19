#ifndef __SERVER__
#define __SERVER__

#pragma once

#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#include "json.hpp"
#include "Common.hpp"
#include "Ticker.h"

namespace server {

using boost::asio::ip::tcp;

struct Transaction {
    size_t id;
    size_t user_id;
    DealData data;
    bool active = true;

    // TODO: заменить конкатинацию на streamstring
    std::string Print() const {
        std::string transaction_data = "\t id:" + std::to_string(id) + ". " \
            + std::to_string(data.dollars) + " dollars for " \
            + std::to_string(data.rubles) + " rubles \n";
        return transaction_data;
    }

    bool IsMatch(const Transaction& other) const {
        if (data.dollars >= other.data.dollars && user_id != other.user_id \
            && data.rubles <= other.data.rubles && active && other.active) {
            return true;
        }
        return false;
    }
};

struct TransactionComparator {
    bool operator() (const Transaction &lhs, const Transaction &rhs) const{
        if (lhs.data.rubles == rhs.data.rubles) {
            return lhs.data.dollars > rhs.data.dollars;
        }
        return lhs.data.rubles > rhs.data.rubles;
    }
};

struct TransactionNOTActive {
    bool operator() (const Transaction &data) const {
        return !data.active;
    }
};

struct User {
    size_t id;
    std::string name;
    std::string pass;
    int dollar_account;
    int rubles_account;
};

class Core
{
public:
    using RequestHandlerFunc = nlohmann::json (server::Core::*)(nlohmann::json);

    Core();
    std::string HandleRequest(nlohmann::json data);
    void TransactionManagment();

private:
    std::unordered_map<size_t, User> id_to_user_;
    std::unordered_map<std::string, size_t> username_to_id_;
    std::unordered_map<std::string_view, RequestHandlerFunc> request_to_executor_function;

    std::vector<Transaction> sell_;
    std::vector<Transaction> buy_;

    // Вход пользователя
    nlohmann::json UserConnection(nlohmann::json message);
    nlohmann::json RegisterNewUser(const std::string& aUserName, const std::string& aUserPass);
    nlohmann::json UserAuthorization(const std::string& aUserName, const std::string& aUserPass);

    // Запросы покупки/продажи
    DealData ParseDealData(nlohmann::json message);
    nlohmann::json CreateBuyTicket(nlohmann::json message);
    nlohmann::json CreateSellTicket(nlohmann::json message);

    // Данные по балансу
    nlohmann::json BalanceData(nlohmann::json message);

    // Активные транзакции данного пользователя
    nlohmann::json UserTransactions(nlohmann::json message);

    // Все активные транзакции
    nlohmann::json ActiveTransactions(nlohmann::json message);

    // Изменить транзакцию
    nlohmann::json ChangeTransaction(nlohmann::json message);

    // Обработка пары транзакций
    void Transact(Transaction& sell, Transaction& buy);
};

Core& GetCore();

class Session
{
public:
    Session(boost::asio::io_service& io_service);
    tcp::socket& GetSocket();
    void Start();

    // Обработка полученного сообщения.
    void HandleRead(const boost::system::error_code& error, size_t bytes_transferred);
    void HandleWrite(const boost::system::error_code& error);

private:
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class Server
{
public:
    Server(boost::asio::io_service& io_service);
    void HandleAccept(Session* new_Session, const boost::system::error_code& error);

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

} // server

#endif