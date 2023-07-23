#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>

#include "misc/readiness/readiness.hh"
#include "server.hh"
#include "utils/utils.hh"
using namespace boost;

int main(int argc, char** argv)
{
    if (argc != 2 && argc != 3)
        return 1;

    // ================== Setup Config ==================
    bool dry_run = argc == 3;
    std::string json_path = dry_run ? argv[2] : argv[1];
    Config::getInstance(json_path);

    // =============== Config Verification ==============
    if (invalid_server_configuration())
        return 1;
    if (dry_run)
        return 0;

    std::clog << "===================== Vhost Info =====================\n";
    Config::getInstance().display_vhosts();

    // ================== Launch Server ==================
    misc::announce_spider_readiness(argv[0]);
    boost::asio::io_service io_service;
    auto servers = std::vector<std::shared_ptr<Server>>();
    for (const auto& vhost : Config::getInstance().get_vhost())
    {
        asio::ip::address raw_ip_address = vhost->getIp();
        unsigned short port_num = vhost->getPort();
        try
        {
            servers.push_back(std::make_shared<Server>(io_service, port_num,
                                                       raw_ip_address, vhost));
        }
        catch (system::system_error& e)
        {
            BOOST_LOG_TRIVIAL(info)
                << "Spider(): Error occurred! Error code = " << e.code()
                << ". Message: " << e.what();
            return e.code().value();
        }
    }
    // ================ Graceful Shutdown ================
    boost::asio::signal_set signal(io_service, SIGINT);
    signal.async_wait([&](auto, auto) {
        BOOST_LOG_TRIVIAL(info) << "Spider(): Graceful Shutdown \n";
        io_service.stop();
    });

    io_service.run();
    return 0;
}