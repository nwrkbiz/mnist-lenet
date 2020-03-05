/**
 * @file WSSObserver.h
 * @brief Class to handle Websocket requests sent by a client.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef WSSOBSERVER_H
#define WSSOBSERVER_H

#include <WebSocketServer.h>
#include <Exception.h>
#include "MNISTLeNet.h"

/**
 * @brief Exception to be thrown on websocket errors.
 */
class WSSObserverException final : public giri::ExceptionBase
{
public:
  WSSObserverException(const std::string &msg) : giri::ExceptionBase(msg) {}; 
  using SPtr = std::shared_ptr<WSSObserverException>;
  using UPtr = std::unique_ptr<WSSObserverException>;
  using WPtr = std::weak_ptr<WSSObserverException>;
};

/**
 * @brief Class to handle Websocket requests sent by a client.
 * 
 * Following JSON is understood by the Observer:
 * 
 * {
 *   "command" : "predict",
 *   "picture" : "base-64-encoded-jpeg"
 * }
 * 
 */
class WSSObserver : 
    public giri::Observer<giri::WebSocketServer>, 
    public giri::Observer<giri::WebSocketSession>, 
    public std::enable_shared_from_this<WSSObserver>
{
public:

    /**
     * @param nw Class containing LeNet used for predictions.
     */
    WSSObserver(const MNISTLeNet::SPtr& nw);

    ~WSSObserver() = default;

    /**
     * @brief On Connect callback.
     */
    void update(giri::WebSocketServer::SPtr serv);

    /**
     * @brief On Message callback, handles client requests.
     */
    void update(giri::WebSocketSession::SPtr sess);

    using SPtr = std::shared_ptr<WSSObserver>;
    using UPtr = std::unique_ptr<WSSObserver>;
    using WPtr = std::weak_ptr<WSSObserver>;
private:
    MNISTLeNet::SPtr m_Network;
};


#endif // WSSOBSERVER_H