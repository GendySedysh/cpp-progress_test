#include "Client.h"

namespace client {

using boost::asio::ip::tcp;


/* -------------------- RequestHandler -------------------- */

RequestHandler::RequestHandler() {
    request_to_executor_function[Requests::BUY] = &RequestHandler::Deal;
    request_to_executor_function[Requests::SELL] = &RequestHandler::Deal;
}

nlohmann::json RequestHandler::PrepareRequest(std::string_view type) {
    nlohmann::json req;

    // нет функций способных обработать запрос
    if (request_to_executor_function.count(type) == 0) {
        req["Code"] = ResponseCode::ERROR;
        return req;
    }

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

    if (!data.IsValid()) {
        req["Code"] = ResponseCode::ERROR;
        return req;
    }

    req["Code"] = ResponseCode::OK;
    req["Dollars"] = data.dollars;
    req["Rubles"] = data.rubles;
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
                 "3) Exit\n"
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
            exit(0);
            break;
        }
        default:
        {
            std::cout << "Unknown menu option\n" << std::endl;
        }
    }
    if (req.at("Message").at("Code") == ResponseCode::ERROR) {
        std::cout << "Invalid request" << std::endl;
        return;
    }
    SendMessage(req);
}

// Отправка сообщения на сервер по шаблону.
void Client::SendMessage(nlohmann::json req) {
    req["UserId"] = id_;

    std::string request = req.dump();
    std::cout << request << std::endl;
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
bool Client::ProcessRegistration() {
    std::string name;
    std::cout << "Hello! Enter your name: ";
    std::cin >> name;

    nlohmann::json req;
    req["ReqType"] = Requests::REG;
    req["Message"] = name;
    // Для регистрации Id не нужен, заполним его нулём
    SendMessage(req);
    std::string response = ReadMessage();
    id_ = response;
    return true;
}

} // client
