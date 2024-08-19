#include "Client.h"

namespace client {

using boost::asio::ip::tcp;


/* -------------------- RequestHandler -------------------- */

RequestHandler::RequestHandler() {
    request_to_executor_function[Requests::BUY] = &RequestHandler::Deal;
    request_to_executor_function[Requests::SELL] = &RequestHandler::Deal;
    request_to_executor_function[Requests::CHANGE] = &RequestHandler::Change;
}

nlohmann::json RequestHandler::PrepareRequest(std::string_view type) {
    nlohmann::json req;

    auto funcIter = request_to_executor_function.at(type);
    req["Message"] = (this->*funcIter)();
    return req;
}

DealData RequestHandler::RequestDealData() {
    DealData data;

    std::cout << "How many dollars? : ";
    std::cin >> data.dollars;
    std::cout << "for how many rubles? : ";
    std::cin >> data.rubles;

    return data;
}

nlohmann::json RequestHandler::Deal() {
    DealData data = RequestDealData();

    nlohmann::json req;

    req["Dollars"] = data.dollars;
    req["Rubles"] = data.rubles;
    return req;
}

nlohmann::json RequestHandler::Change() {
    size_t transaction_id;
    std::cout << "Enter transaction ID to change : ";
    std::cin >> transaction_id;

    nlohmann::json req = Deal();
    req["TransactionId"] = transaction_id;
    return req;
}

/* -------------------- Client -------------------- */

Client::Client() {
    tcp::resolver resolver(io_);
    tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
    tcp::resolver::iterator iterator = resolver.resolve(query);

    socket_.connect(*iterator);
}

void Client::AskRequest() {
    std::cout << "Menu:\n"
                 "1) Sell\n"
                 "2) Buy\n"
                 "3) Check balance\n"
                 "4) My active transactions\n"
                 "5) Change transaction\n"
                 "6) All active transactions\n"
                 "7) Exit\n"
                 << std::endl;

    short menu_option_num;
    std::cin >> menu_option_num;

    nlohmann::json req;

    switch (menu_option_num)
    {
        case 1:
        {
            req = request_handler_.PrepareRequest(Requests::SELL);
            req["ReqType"] = Requests::SELL;
            break;
        }
        case 2:
        {
            req = request_handler_.PrepareRequest(Requests::BUY);
            req["ReqType"] = Requests::BUY;
            break;
        }
        case 3:
        {
            req["ReqType"] = Requests::BALANCE;
            break;
        }
        case 4:
        {
            req["ReqType"] = Requests::USER_ACTIVE;
            break;
        }
        case 5:
        {
            req = request_handler_.PrepareRequest(Requests::CHANGE);
            req["ReqType"] = Requests::CHANGE;
            break;
        }
        case 6:
        {
            req["ReqType"] = Requests::ACTIVE;
            break;
        }
        case 7:
        {
            exit(0);
            break;
        }
        default:
        {
            std::cout << "Unknown menu option\n" << std::endl;
            return;
        }
    }
    SendMessage(req);
    std::string response = ReadMessage();
    auto json = nlohmann::json::parse(response);
    std::cout << json.at("Message").get<std::string>() << std::endl;
}

// Отправка сообщения на сервер по шаблону.
void Client::SendMessage(nlohmann::json req) {
    req["Message"]["UserId"] = id_;

    std::string request = req.dump();
    boost::asio::write(socket_, boost::asio::buffer(request, request.size()));
}

// Возвращает строку с ответом сервера на последний запрос.
std::string Client::ReadMessage() {
    boost::asio::streambuf b;
    boost::asio::read_until(socket_, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return line;
}

// "Создаём" пользователя, получаем его ID.
bool Client::ProcessAuthorization() {
    std::string name, pass;
    std::cout << "Hello! Enter your name: ";
    std::cin >> name;
    std::cout << "password: ";
    std::cin >> pass;

    nlohmann::json message;
    message["Username"] = name;
    message["Password"] = pass;

    nlohmann::json req;
    req["ReqType"] = Requests::REG;
    req["Message"] = message;

    SendMessage(req);
    std::string response = ReadMessage();
    nlohmann::json json = nlohmann::json::parse(response.c_str());

    auto code = json["Code"];

    std::cout << json["Message"] << std::endl;
    if (code == ResponseCode::ERROR) {
        return false;
    }

    id_ = json["Id"];
    return true;
}

} // client
