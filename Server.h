#ifndef _INC_SERVER_H
#define _INC_SERVER_H

#include <memory>
#include <string>
#include "Socket.h"
/*
 * TCP Server
 */
class Server
{
	// Handler to be used on connection
	using HandlerType = void(*)(Socket&);
	HandlerType handler = nullptr;

	public:
		Server() = default;

		// Listen on port `port`
		// Continuously execute handler on received data
		Server& listen(int port);

		Server& setHandler(HandlerType _handler){handler=_handler;return *this;}
};
#endif
