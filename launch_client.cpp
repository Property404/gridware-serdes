#include "launch.h"
#include "Server.h"
#include "Socket.h"
#include "Serializer.h"
#include <fstream>
#include <sstream>
#include <iostream>

static void receiveFirmwareImage(const Socket& connection)
{
	// Load from socket into firmware image page object
	gridware_FirmwareImagePage obj;

	{
		SerializerBuffer<decltype(obj)> buffer;
		buffer.setLength(
			connection.receive(buffer.getData(), buffer.getDataSize())
		);
		if(buffer.deserialize(obj))
			throw std::runtime_error("Failed to deserialize message");
	}

	// Check if CRC is OK
	std::cerr<<"CRC: "<<static_cast<int>(obj.crc.bytes[0])<<std::endl;
	// Send verify
	{
		std::cerr<<"Adjust this response code"<<std::endl;
		gridware_DeviceResponse response = gridware_DeviceResponse_init_zero;
		response.verified = true;
		std::cout<<"Freakin'"<<std::endl;
		const SerializerBuffer<decltype(response)> buffer(response);
		std::cout<<"Sendin'"<<std::endl;
		connection.send(buffer.getData(), buffer.getLength());
	}

	if(!obj.last)
		receiveFirmwareImage(connection);
}
void launchClient(int port)
{
	Socket socket;
	socket.connect("localhost", port);
	receiveFirmwareImage(socket);
}
