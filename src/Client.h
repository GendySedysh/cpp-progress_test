#ifndef __CLIENT__
#define __CLIENT__

#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <unordered_map>

#include "Common.hpp"
#include "json.hpp"

namespace client {

using boost::asio::ip::tcp;

class RequestHandler {
public:
    using RequestHandlerFunc = nlohmann::json (client::RequestHandler::*)();

    RequestHandler();
    nlohmann::json PrepareRequest(std::string_view type);
private:
    std::unordered_map<std::string_view, RequestHandlerFunc> request_to_executor_function;

    DealData RequestDealData();
    nlohmann::json Deal();
};

class Client {
public:
    Client();

    bool ProcessRegistration();
    void AskRequest();

private:
    boost::asio::io_service io_;
    tcp::socket socket_{io_};
    RequestHandler request_handler_;
    std::string id_ = "0";

    void SendMessage(nlohmann::json req);
    std::string ReadMessage();
};

} // client

#endif