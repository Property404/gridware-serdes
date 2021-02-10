#include <catch2/catch.hpp>

#include "Serializer.h"
#include "Server.h"

#include <cstdlib>
#include <iostream>
#include <thread>

TEST_CASE("Serialize-deserialize DeviceResponse", "[gwSerializeMessage][gwDeserializeMessage]")
{
	for(std::uint8_t i=0;i<=100;i++)
	{
		SerializerBuffer<gridware_DeviceResponse> buffer;
		
		gridware_DeviceResponse message_original;
		message_original.verified = !!(i%2);

		REQUIRE(0==buffer.serialize(message_original));

		gridware_DeviceResponse message_decoded;
		REQUIRE(0==buffer.deserialize(message_decoded));

		REQUIRE(message_original.verified == message_decoded.verified);
	}
}

TEST_CASE("Serialize-deserialize FirmwareImagePage", "[gwSerializeMessage][gwDeserializeMessage]")
{
	for(std::uint8_t i=0;i<=100;i++)
	{
		
		SerializerBuffer<gridware_FirmwareImagePage> buffer;
		gridware_FirmwareImagePage message_original = gridware_FirmwareImagePage_init_zero;

		// Randomize message
		message_original.last = !!(rand()%2);
		for(unsigned j=0;j<sizeof(message_original.page.bytes);j++)
			message_original.page.bytes[j] = rand()%256;
		message_original.page.size = sizeof(message_original.page.bytes);
		message_original.crc.bytes[0] = rand()%256;
		message_original.crc.bytes[1] = rand()%256;
		message_original.crc.size = 2;

		// Serialize message
		REQUIRE(0==buffer.serialize(message_original));

		// Deserialize
		gridware_FirmwareImagePage message_decoded;
		REQUIRE(0==buffer.deserialize(message_decoded));

		// Make sure deserialized message is same as original
		REQUIRE(message_original.last == message_decoded.last);
		for(unsigned j=0;j<message_original.page.size;j++)
			REQUIRE(message_decoded.page.bytes[j] == 
					message_original.page.bytes[j]);

	}
}


TEST_CASE("Socket send and receive")
{
	constexpr auto port = 8993;
	const unsigned char sentence[] = "I am the dream you chase";
	
	// Send sentence to client then hang up
	std::thread server_thread([port](){
		Server server;
		server.setHandler([](Socket& connection)->void{
			
			// Send test vector
			const unsigned char sentence[] = "I am the dream you chase";
			connection.send(static_cast<const uint8_t*>(sentence), sizeof(sentence));

			// Wait for quit signal
			uint8_t val;
			connection.receive(&val, sizeof(val));
		}).listen(port);
	});

	// Receive on client
	std::thread client_thread([&sentence, port](){
		Socket client;
		client.connect("localhost", port);
		
		// Grab test vector
		unsigned char buffer[64]={0};
		client.receive(static_cast<uint8_t*>(buffer), sizeof(buffer));

		// Verify
		const auto expected =
			std::string(reinterpret_cast<const char*>(sentence));
		const auto actual = 
			std::string(reinterpret_cast<const char*>(buffer));
		REQUIRE(actual == expected);
		
		// Send quit signal
		uint8_t val;
		client.send(&val, 1);
	});

	client_thread.join();
	server_thread.join();
}

TEST_CASE("Serialize, send, receive, deserialize")
{
	constexpr auto port = 8994;
	
	// Send sentence to client then hang up
	std::thread server_thread([port](){
		Server server;
		server.setHandler([](Socket& connection)->void{
			// Serialize data
			gridware_FirmwareImagePage obj = gridware_FirmwareImagePage_init_zero;
			obj.crc.bytes[0] = 0xCA;
			obj.crc.bytes[1] = 0xFE;
			obj.crc.size = 2;
			const SerializerBuffer<decltype(obj)> buffer(obj);

			// Send data
			REQUIRE(obj.crc.bytes[1] == 0xFE);
			{
				decltype(obj) clone = gridware_FirmwareImagePage_init_zero;
				buffer.deserialize(clone);
				REQUIRE(clone.crc.bytes[0] == obj.crc.bytes[0]);
				REQUIRE(clone.crc.bytes[1] == obj.crc.bytes[1]);
				REQUIRE(clone.crc.size == obj.crc.size);
			}
			connection.send(buffer.getData(), buffer.getLength());

			// Wait for quit signal
			uint8_t val;
			connection.receive(&val, sizeof(val));
		}).listen(port);
	});

	// Receive on client
	std::thread client_thread([port](){
		Socket client;
		client.connect("localhost", port);

		// receive and deserialize
		gridware_FirmwareImagePage obj = gridware_FirmwareImagePage_init_zero;
		SerializerBuffer<decltype(obj)> buffer;
		buffer.setLength(client.receive(buffer.getData(), buffer.getDataSize()));
		buffer.deserialize(obj);

		// Verify
		REQUIRE(obj.crc.bytes[0] == 0xCA);
		REQUIRE(obj.crc.bytes[1] == 0xFE);
		
		// Send quit signal
		uint8_t val;
		client.send(&val, 1);
	});

	client_thread.join();
	server_thread.join();
}
