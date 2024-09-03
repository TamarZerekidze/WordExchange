//
// Created by takusi on 9/2/2024.
//

#ifndef HANDLER_H
#define HANDLER_H

#include <string>

// Abstract Handler Class
class Handler {
protected:
    Handler *nextHandler = nullptr;
public:
    virtual ~Handler() = default;
    virtual void setNext(Handler *handler) {
        nextHandler = handler;
    }
    virtual int handle(SOCKET client_socket, const std::string &username) {
        if (nextHandler) {
            return nextHandler->handle(client_socket, username);
        }
        return 0;
    }
};


#endif //HANDLER_H
