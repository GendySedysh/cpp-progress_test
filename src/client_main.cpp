#include "Client.h"

using namespace client;
using boost::asio::ip::tcp;

int main()
{
    try
    {
        boost::asio::io_service io_service;
        Client client;

        if (!client.ProcessAuthorization()) {
            exit(0);
        }

        while (true)
        {
           client.AskRequest();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}