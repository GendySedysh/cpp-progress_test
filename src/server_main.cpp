#include "Server.h"

using namespace server;

int main()
{
    try
    {
        boost::asio::io_service io_service;

        boost::asio::signal_set signals(io_service, SIGINT, SIGTERM);
        signals.async_wait([&io_service](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                io_service.stop();
                std::cout << std::endl << "Shutdown server" << std::endl;
            }
        });

        static Core core;
        Server s(io_service);

        // Сервер в фоновом режиме раз в N миллисекунд будет проводить торговлю по активным заявкам 
        auto ticker = std::make_shared<Ticker>(io_service, std::chrono::milliseconds(TICKRATE),
            [](std::chrono::milliseconds delta) { GetCore().TransactionManagment(); }
        );
        ticker->Start();

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}