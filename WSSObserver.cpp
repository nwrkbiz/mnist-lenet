/**
 * @file WSSObserver.cpp
 * @brief Class to handle Websocket requests sent by a client.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#include <JSON.h>
#include <Blob.h>
#include "WSSObserver.h"

using namespace giri;

WSSObserver::WSSObserver(const MNISTLeNet::SPtr& nw) : m_Network(nw) {
}

void WSSObserver::update(WebSocketServer::SPtr serv){
    serv->getSession()->subscribe(this->shared_from_this()); // subscribe this observer to the session
}

void WSSObserver::update(WebSocketSession::SPtr sess){
 
    if(sess->getError())
        return;

    json::JSON answ;
    try{
        std::error_code ec;
        json::JSON msg = json::JSON::Load(sess->getMessage(), ec);

        if(ec)
            throw WSSObserverException("Invalid request sent! Could not parse JSON.");
        else if(!msg.hasKey("command"))
            throw WSSObserverException("Invalid request sent! JSON is missing the command field.");
        else
        {
            if(msg["command"].ToString() == "predict"){
                if(!msg.hasKey("picture"))
                    throw WSSObserverException("Invalid request sent! JSON is missing the picture field.");

                Blob pic;
                pic.loadBase64(msg["picture"].ToString());
                answ["result"] = m_Network->predict(pic);
                answ["state"] = "ok";
                sess->send(answ.ToString());
                return;
            }
        }
        throw WSSObserverException("Unknown request received.");        
    }
    catch(const ExceptionBase& e)
    {
        answ["state"] = "Error";
        answ["message"] = e.getMessage();
        sess->send(answ.ToString());
    }
    catch (...) // catch everything to keep the service running
    {
        answ["state"] = "Error";
        answ["message"] = "Unknown error occured.";
        sess->send(answ.ToString()); 
    }
}