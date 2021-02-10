/* Simulate the transfer of binary firmware image */
#include "launch.h"
#include <thread>
#include <iostream>
constexpr int PORT = 8113;

int main()
{
	std::cout<<"Launching server"<<std::endl;
	auto server_thread = std::thread(launchServer,PORT);

	std::cout<<"Launching client"<<std::endl;
	auto client_thread = std::thread(launchClient,PORT);

	client_thread.join();
	server_thread.join();
	std::cout<<"Simulation finished"<<std::endl;
}
