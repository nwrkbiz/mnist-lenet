/**
 * @file main.cpp
 * @brief Program entry point.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#include <HTTPServer.h>
#include <WebSocketServer.h>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include "MNISTLeNet.h"
#include "WSSObserver.h"
using namespace giri;
using namespace std;
namespace po = boost::program_options;

int main(int argc, char** argv)
{
    try{
        // commandline options
        po::options_description desc("Allowed options");
        desc.add_options()
        ("help", "Print help.")
        ("cert", po::value<std::string>(), "Certificate file, if provided this file is used for tls encryption of the websocket and http connections.")
        ("key", po::value<std::string>(), "Private key for the certificate file, if provided this file is used for tls encryption of the websocket and http connections.")
        ("httpport", po::value<std::string>(), "Port to listen for HTTP connections. (defaults to 8808)")
        ("wssport", po::value<std::string>(), "Port to listen for WebSocket connections. (defaults to 8809)")
        ("mnist", po::value<std::string>(), "Path to folder which contains the mnist dataset. (defaults to ./mnist)")
        ("client", po::value<std::string>(), "Path to folder which contains the HTML5 client. (defaults to ./client)");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);    

        // print help message
        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        // check for certificates
        std::filesystem::path certFile;
        std::filesystem::path keyFile;
        bool ssl = false;
        if(vm.count("cert") && vm.count("key") ){
            certFile = vm["cert"].as<std::string>();
            keyFile = vm["key"].as<std::string>();
            if(filesystem::exists(certFile) && filesystem::exists(keyFile))
                ssl = true;
        }

        // check for ports
        std::string httpPort = "8808";
        std::string wssPort = "8809";
        if(vm.count("httpport"))
            httpPort = vm["httpport"].as<std::string>();
        if(vm.count("wssport"))
            wssPort = vm["wssport"].as<std::string>();

        // path to mnist folder
        std::filesystem::path mnistPath = "mnist";
        if(vm.count("mnist"))
            mnistPath = vm["mnist"].as<std::string>();

        // path to html5 client
        std::filesystem::path clientPath = "client";
        if(vm.count("client"))
            mnistPath = vm["client"].as<std::string>();

        // create network instance, ctor will automatically train a new network if none exists
        MNISTLeNet::SPtr network = std::make_shared<MNISTLeNet>(mnistPath);

        // websocket server for client interaction
        WSSObserver::SPtr obs = std::make_shared<WSSObserver>(network); // observer handling requests
        WebSocketServer::SPtr wssrv = std::make_shared<WebSocketServer>("0.0.0.0", wssPort, ssl , 2, certFile, keyFile);
        wssrv->subscribe(obs);
        wssrv->run();

        // HTTPServer serving HTML5 client
        HTTPServer::SPtr httpserver = std::make_shared<HTTPServer>("0.0.0.0", httpPort, clientPath, 2, ssl, certFile, keyFile);
        httpserver->run();


        // Startup Info
        std::string host = boost::asio::ip::host_name();
        std::string httpprotocol = "http://";
        if(ssl)
            std::string httpprotocol = "https://";
        std::string wsprotocol = "ws://";
        if(ssl)
            std::string wsprotocol = "wss://";
        std::cout << "Service running. Navigate your browser to: " << httpprotocol << host << ":" << httpPort  << std::endl;
        std::cout << "Websocket service URI: " << wsprotocol << host << ":" << wssPort << std::endl;

        // exit by command:
        std::string exit;
        while(exit != "exit"){
            std::cout << "Enter exit to stop the program: " << std::endl;
            std::cin >> exit;
        };
    }
    catch(const ExceptionBase& e){
        std::cerr << e.getMessage() << std::endl;
    }
    return EXIT_SUCCESS;
}