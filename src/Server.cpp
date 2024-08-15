#include "Server.h"

namespace server {
    
using boost::asio::ip::tcp;

Core& GetCore()
{
    static Core core;
    return core;
}

/* -------------------- Core -------------------- */

// "Регистрирует" нового пользователя и возвращает его ID.
std::string Core::RegisterNewUser(const std::string& aUserName) {
    if (usernames_.count(aUserName) != 0) {
        return "Error! Username occupied";
    }

    size_t newUserId = id_to_user_.size();
    id_to_user_[newUserId] = { newUserId, aUserName, 0, 0 };
    usernames_.insert(aUserName);

    return std::to_string(newUserId);
}

// Запрос имени клиента по ID
std::string Core::GetUserName(const std::string& aUserId) {
    const auto user_id = std::stoi(aUserId);
    if (id_to_user_.count(user_id) == 0) {
        return "Error! Unknown User";
    }
    return id_to_user_.at(user_id).name;
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
        auto reqType = j["ReqType"];

        std::string reply = "Error! Unknown request type";
        if (reqType == Requests::Registration)
        {
            // Это реквест на регистрацию пользователя.
            // Добавляем нового пользователя и возвращаем его ID.
            std::cout << "register" << std::endl;
            reply = GetCore().RegisterNewUser(j["Message"]);
        }
        else if (reqType == Requests::Hello)
        {
            // Это реквест на приветствие.
            // Находим имя пользователя по ID и приветствуем его по имени.
            std::cout << "hello" << std::endl;
            reply = "Hello, " + GetCore().GetUserName(j["UserId"]) + "!\n";
        }

        boost::asio::async_write(socket_,
            boost::asio::buffer(reply, reply.size()),
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