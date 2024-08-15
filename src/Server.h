#ifndef __SERVER__
#define __SERVER__

#pragma once

#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <set>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include "json.hpp"
#include "Common.hpp"

namespace server {

using boost::asio::ip::tcp;

struct User {
    size_t id;
    std::string name;
    int dollar_account;
    int rubles_account;
};

class Core
{
public:
    std::string RegisterNewUser(const std::string& aUserName);
    std::string GetUserName(const std::string& aUserId);

private:
    // <UserId, User>
    std::unordered_map<size_t, User> id_to_user_;
    std::set<std::string> usernames_;
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