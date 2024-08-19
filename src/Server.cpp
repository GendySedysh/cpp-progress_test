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
    request_to_executor_function[Requests::CHANGE] = &Core::ChangeTransaction;
}

std::string Core::HandleRequest(nlohmann::json data) {
    auto type = data["ReqType"].get<std::string>();

    if (request_to_executor_function.count(type) == 0) {
        nlohmann::json resp;
        resp["Code"] = ResponseCode::ERROR;
        resp["Message"] = "Error! Command is not valid";
        return resp.dump();
    }

    auto funcIter = request_to_executor_function.at(type);
    nlohmann::json response = (this->*funcIter)(data.at("Message"));
    return response.dump();
}

void Core::TransactionManagment() {
    // нечего обрабатывать
    if (sell_.empty() || buy_.empty()) {
        return ;
    }

    std::sort(sell_.begin(), sell_.end(), TransactionComparator());
    std::sort(buy_.begin(), buy_.end(), TransactionComparator());

    for (auto &sell_trans: sell_) {
        for (auto &buy_trans: buy_) {
            if (sell_trans.IsMatch(buy_trans)) {
                Transact(sell_trans, buy_trans);
            }
        }
    }

    sell_.erase(
        std::remove_if(sell_.begin(), sell_.end(), TransactionNOTActive()), sell_.end()
    );

    buy_.erase(
        std::remove_if(buy_.begin(), buy_.end(), TransactionNOTActive()) , buy_.end()
    );
}

void Core::Transact(Transaction& sell, Transaction& buy) {
    User& seller = id_to_user_.at(sell.user_id);
    User& buyer = id_to_user_.at(buy.user_id);

    seller.dollar_account -= buy.data.dollars;
    seller.rubles_account += buy.data.dollars * buy.data.rubles;

    buyer.dollar_account += buy.data.dollars;
    buyer.rubles_account -= buy.data.dollars * buy.data.rubles;

    buy.active = false;
    sell.data.dollars -= buy.data.dollars;
    if (sell.data.dollars == 0) {
        sell.active = false;
    }
}

nlohmann::json Core::UserConnection(nlohmann::json message) {
    auto aUserName = message.at("Username").get<std::string>();
    auto aUserPass = message.at("Password").get<std::string>();

    if (aUserName.empty() || aUserPass.empty()) {
        nlohmann::json resp;
        resp["Code"] = ResponseCode::ERROR;
        resp["Message"] = "Error! Empty name or password";
        return resp;
    }

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
    buy_.push_back({transaction_id, user_id, deal_data});

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
    sell_.push_back({transaction_id, user_id, deal_data});

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

    std::string data = "Your active transactions\n";
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

nlohmann::json Core::ChangeTransaction(nlohmann::json message) {
    size_t transaction_id = message["TransactionId"].get<int>();

    auto transaction = std::find_if(sell_.begin(), sell_.end(), [&transaction_id](const Transaction &data) {
        return data.id == transaction_id;
    });

    if (transaction == sell_.end()) {
        transaction = std::find_if(buy_.begin(), buy_.end(), [&transaction_id](const Transaction &data) {
            return data.id == transaction_id;
        });
    }

    nlohmann::json resp;
    resp["Code"] = ResponseCode::ERROR;
    if (transaction == sell_.end()) {
        resp["Message"] = "Error! No active transaction with ID " + std::to_string(transaction_id);
        return resp;
    }

    size_t user_id = message["UserId"].get<int>();
    if (transaction->user_id != user_id) {
        resp["Message"] = "Error! You dont have premissions to change transaction with ID " + std::to_string(transaction_id);
        return resp;
    }

    DealData deal_data = ParseDealData(message);
    if (!deal_data.IsValid()) {
        resp["Message"] = "Error! Transaction is not valid";
        return resp;
    }

    transaction->data = deal_data;
    resp["Code"] = ResponseCode::OK;
    resp["Message"] = "Transaction data changed";
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
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
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