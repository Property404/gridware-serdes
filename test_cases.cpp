#include <catch2/catch.hpp>
#include <cstdlib>
#include <iostream>
#include "Serializer.h"

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
