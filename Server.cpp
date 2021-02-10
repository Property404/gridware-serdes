#include "Server.h"
#include "Socket.h"
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <thread>

Server& Server::listen(int port)
{
	Socket listener;
	listener.bind(port);
	listener.listen();

	// Wait for a connection
	do
	{
		Socket connection(listener.accept());

		if(!connection.isValid())
		{
			std::cerr<<"Failed to accept incoming connection"<<std::endl;
			continue;
		}


		const auto local_handler = handler;
		std::thread([&local_handler, &connection]()->void
		{
			// Newborn child acts does some stuff with client
			// then kills itself

			try {local_handler(connection);}
			catch(const std::exception& e){
				std::cerr<<"Exception in server event loop: "<<e.what()<<std::endl;
			}
		}).join();
	}while(false);// Only iterate once, because we want to close the server after testing
	// and this is just a simulation
	//
	// In real life, we'd want to keep accepting connections
	return *this;
}
