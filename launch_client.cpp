#include "launch.h"
#include "Server.h"
#include "Socket.h"
#include "Serializer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "checksum.h"
#include <unistd.h>

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
	const uint16_t checksum = crc_16(obj.page.bytes, obj.page.size);
	const bool valid = (checksum == ((obj.crc.bytes[1]<<8) | obj.crc.bytes[0]));
	if(valid)
		std::cout<<"Client: Checksum OK!"<<std::endl;
	else
		std::cout<<"Client: Checksum verification failed";

	// Send verify
	{
		gridware_DeviceResponse response = gridware_DeviceResponse_init_zero;
		response.verified = valid;
		const SerializerBuffer<decltype(response)> buffer(response);
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
