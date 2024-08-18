#include "Server.h"

namespace server {
    
using boost::asio::ip::tcp;

Core& GetCore()
{
    static Core core;
    return core;
}

/* -------------------- Core -------------------- */
Core::Core() {
    request_to_executor_function[Requests::REG] = &Core::UserConnection;
    request_to_executor_function[Requests::BUY] = &Core::CreateBuyTicket;
    request_to_executor_function[Requests::SELL] = &Core::CreateSellTicket;
    request_to_executor_function[Requests::BALANCE] = &Core::BalanceData;
    request_to_executor_function[Requests::USER_ACTIVE] = &Core::UserTransactions;
    request_to_executor_function[Requests::ACTIVE] = &Core::ActiveTransactions;
}

std::string Core::HandleRequest(nlohmann::json data) {
    auto type = data["ReqType"].get<std::string>();

    auto funcIter = request_to_executor_function.at(type);
    nlohmann::json response = (this->*funcIter)(data.at("Message"));
    return response.dump();
}

nlohmann::json Core::UserConnection(nlohmann::json message) {
    auto aUserName = message.at("Username").get<std::string>();
    auto aUserPass = message.at("Password").get<std::string>();
    // Существует ли пользователь с данным именем
    if (username_to_id_.count(aUserName) == 0) {
        return RegisterNewUser(aUserName, aUserPass);
    }
    return UserAuthorization(aUserName, aUserPass);
}

// "Авторизует" существующего пользователя, возвращая его ID
nlohmann::json Core::UserAuthorization(const std::string& aUserName, const std::string& aUserPass) {
    nlohmann::json resp;
    size_t user_id = username_to_id_.at(aUserName);

    if (id_to_user_.at(user_id).pass != aUserPass) {
        resp["Code"] = ResponseCode::ERROR;
        resp["Message"] = "Error! Wrong username or password";
        return resp;
    }

    resp["Code"] = ResponseCode::OK;
    resp["Id"] = user_id;
    resp["Message"] = "Authentication successfully completed";
    return resp;
}

// "Регистрирует" нового пользователя и возвращает его ID.
nlohmann::json Core::RegisterNewUser(const std::string& aUserName, const std::string& aUserPass) {
    nlohmann::json resp;

    size_t newUserId = id_to_user_.size();
    id_to_user_[newUserId] = { newUserId, aUserName, aUserPass, 0, 0 };
    username_to_id_[aUserName] = newUserId;

    resp["Code"] = ResponseCode::OK;
    resp["Id"] = newUserId;
    resp["Message"] = "New user successfully registered";
    return resp;
}

DealData Core::ParseDealData(nlohmann::json message) {
    DealData deal{ message["Dollars"].get<int>(), message["Rubles"].get<int>() };
    return deal;
}

nlohmann::json Core::CreateBuyTicket(nlohmann::json message) {
    size_t user_id = message["UserId"].get<int>();
    auto deal_data = ParseDealData(message);

    nlohmann::json resp;
    if (!deal_data.IsValid()) {
        nlohmann::json resp;
        resp["Code"] = ResponseCode::ERROR;
        resp["Message"] = "Error! Transaction is not valid";
        return resp;
    }
    auto transaction_id = sell_.size() + buy_.size() + 1;
    buy_.insert({transaction_id, user_id, deal_data});

    resp["Code"] = ResponseCode::OK;
    resp["Message"] = "Transaction BUY " + std::to_string(deal_data.dollars) + " dollars for " + \
                      std::to_string(deal_data.rubles) + " rubles accepted";
    return resp;
}

nlohmann::json Core::CreateSellTicket(nlohmann::json message) {
    size_t user_id = message["UserId"].get<int>();
    DealData deal_data = ParseDealData(message);

    nlohmann::json resp;
    if (!deal_data.IsValid()) {
        nlohmann::json resp;
        resp["Code"] = ResponseCode::ERROR;
        resp["Message"] = "Error! Transaction is not valid";
        return resp;
    }
    auto transaction_id = sell_.size() + buy_.size() + 1;
    sell_.insert({transaction_id, user_id, deal_data});

    resp["Code"] = ResponseCode::OK;
    resp["Message"] = "Transaction SELL " + std::to_string(deal_data.dollars) + " dollars for " + \
                      std::to_string(deal_data.rubles) + " rubles accepted";
    return resp;
}

nlohmann::json Core::BalanceData(nlohmann::json message) {
    size_t user_id = message["UserId"].get<int>();
    auto user = id_to_user_.at(user_id);

    nlohmann::json resp;
    resp["Code"] = ResponseCode::OK;
    resp["Message"] = "Current balanse is " + std::to_string(user.dollar_account) + " dollars and " + \
                      std::to_string(user.rubles_account) + " rubles";
    return resp;
}

nlohmann::json Core::UserTransactions(nlohmann::json message) {
    size_t user_id = message["UserId"].get<int>();

    nlohmann::json resp;
    resp["Code"] = ResponseCode::OK;

    std::string data = "All active transactions\n";
    data += "to SELL:\n";
    for (const auto transaction: sell_){
        if (transaction.user_id == user_id) {
            data += transaction.Print();
        }
    }

    data += "to BUY:\n";
    for (const auto transaction: buy_){
        if (transaction.user_id == user_id) {
            data += transaction.Print();
        }
    }

    resp["Message"] = data;
    return resp;
}

nlohmann::json Core::ActiveTransactions(nlohmann::json message) {
    nlohmann::json resp;
    resp["Code"] = ResponseCode::OK;

    std::string data = "All active transactions\n";
    data += "to SELL:\n";
    for (const auto transaction: sell_){
        data += transaction.Print();
    }

    data += "to BUY:\n";
    for (const auto transaction: buy_){
        data += transaction.Print();
    }

    resp["Message"] = data;
    return resp;
}


/* -------------------- Session -------------------- */

Session::Session(boost::asio::io_service& io_service)
        : socket_(io_service) {}

tcp::socket& Session::GetSocket() {
    return socket_;
}

void Session::Start() {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&Session::HandleRead, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

// Обработка полученного сообщения.
void Session::HandleRead(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error)
    {
        data_[bytes_transferred] = '\0';

        // Парсим json, который пришёл нам в сообщении.
        auto j = nlohmann::json::parse(data_);

        std::string reply = GetCore().HandleRequest(j);
        
        boost::asio::async_write(socket_,
            boost::asio::buffer(reply.c_str(), reply.size()),
            boost::bind(&Session::HandleWrite, this,
                boost::asio::placeholders::error));
    }
    else
    {
        delete this;
    }
}

void Session::HandleWrite(const boost::system::error_code& error) {
    if (!error)
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&Session::HandleRead, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        delete this;
    }
}

/* -------------------- Server -------------------- */

Server::Server(boost::asio::io_service& io_service)
    : io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "Server started! Listen " << port << " port" << std::endl;

    Session* new_Session = new Session(io_service_);
    acceptor_.async_accept(new_Session->GetSocket(),
        boost::bind(&Server::HandleAccept, this, new_Session,
            boost::asio::placeholders::error));
}

void Server::HandleAccept(Session* new_Session, const boost::system::error_code& error) {
    if (!error)
    {
        new_Session->Start();
        new_Session = new Session(io_service_);
        acceptor_.async_accept(new_Session->GetSocket(),
            boost::bind(&Server::HandleAccept, this, new_Session,
                boost::asio::placeholders::error));
    }
    else {
        delete new_Session;
    }
}

} // server