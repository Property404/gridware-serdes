#include "launch.h"
#include "Server.h"
#include "Socket.h"
#include "Serializer.h"
#include <fstream>
#include <sstream>
#include <iostream>

static void sendFile(const Socket& connection, const std::string& filename)
{
	// Fetch file contents
	std::string contents;
	{
		std::ifstream fp(filename, std::ios::binary|std::ios::in);
		std::stringstream buffer;
		if(fp.fail()){
			throw std::runtime_error("Could not open file "+filename);
		}
		buffer << fp.rdbuf();
		contents = buffer.str();
		fp.close();
	}

	// Send one FirmwareImagePage per 2048 bytes, or whatever
	const ssize_t max_page_size = sizeof(((gridware_FirmwareImagePage*)nullptr)->page.bytes);
	ssize_t bytes_left = contents.size();
	size_t offset = 0;
	while(bytes_left)
	{
		{
			const auto local_page_size = std::min(max_page_size, bytes_left);
			bytes_left-=local_page_size;
			std::cout<<"Bytes_left: "<<bytes_left<<std::endl;

			gridware_FirmwareImagePage obj = gridware_FirmwareImagePage_init_zero;

			obj.last = (bytes_left <= max_page_size);
			std::cout<<"LAST!"<<std::endl;

			obj.page.size = local_page_size;
			for(unsigned i=0;i<local_page_size;i++)
			{
				obj.page.bytes[i] = contents[offset+i];
			}
			offset+=local_page_size;
			// TODO: CRC
			//
			obj.crc.bytes[0]=0xCA;
			obj.crc.bytes[1]=0xFE;
			obj.crc.size = 2;

			const SerializerBuffer<decltype(obj)> buffer(obj);
			std::cout<<"Sending firmware image page"<<std::endl;
			connection.send(buffer.getData(), buffer.getLength());
		}

		// Receive verification
		{
			gridware_DeviceResponse obj;
			SerializerBuffer<decltype(obj)> buffer;
			buffer.setLength(
					connection.receive(buffer.getData(), buffer.getDataSize())
			);
			if(buffer.deserialize(obj))
				throw std::runtime_error("Could not deserialize DeviceResponse");

			if(obj.verified)
				std::cout<<"Device response A-OK"<<std::endl;
			else
				std::cerr<<"Device response not OK"<<std::endl;
		}
	}
}

void launchServer(int port)
{
	Server server;
	server.setHandler([](Socket& connection)->void {
		sendFile(connection, "binary");
	}).listen(port);
}
