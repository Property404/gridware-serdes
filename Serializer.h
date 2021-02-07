#ifndef _INC_SERIALIZER_H
#define _INC_SERIALIZER_H

#include <pb_encode.h>
#include <pb_decode.h>
#include "objects.pb.h"
#include <type_traits>

// Use this determine the size of SerializerBuffer's
// internal array and such
template<class T>
constexpr size_t _serializer_sizeOfType()
{
	return
		std::is_same<T, gridware_DeviceResponse>::value?
			2:
		std::is_same<T, gridware_FirmwareImagePage>::value?
			2060:
		// Throw compiler error if type unknown
		-1;
}


// Buffer we load our message to/from
// Size is user-controllable, but has a 
// sensible default
template<class T>
class SerializerBuffer
{
	// Actual data
	uint8_t data[_serializer_sizeOfType<T>()];

	// Fixed size of our data
	static constexpr size_t data_size = _serializer_sizeOfType<T>();

	// This is filled in by the encoder function
	size_t message_length;

	// Get specific PB fields object associated with type
	// Used for calling the pb_encode() and pb_decode() functions
	//
	// Usage: _serialize_fieldsOfType<T>()
	static constexpr decltype(gridware_DeviceResponse_fields) fieldsOfType()
	{
		return
			std::is_same<T, gridware_DeviceResponse>::value?
				gridware_DeviceResponse_fields:
			std::is_same<T, gridware_FirmwareImagePage>::value?
				gridware_FirmwareImagePage_fields:
			nullptr;
	}
public:

	SerializerBuffer(){};
	SerializerBuffer(const T& msg){serialize(msg);};

	// Serialize a message into the buffer
	int serialize(const T& message)
	{
		pb_ostream_t stream = pb_ostream_from_buffer(data, data_size);

		const auto status = 
			pb_encode(&stream, fieldsOfType() ,&message);

		message_length = stream.bytes_written;

		if(!status)
			return -1;
		return 0;
	}

	// Deserialize into message
	int deserialize(T& message) const
	{
		pb_istream_t stream = pb_istream_from_buffer(data, message_length);

		const auto status = 
			pb_decode(&stream, fieldsOfType() ,&message);

		if(!status)
			return -1;
		return 0;
	}

	uint8_t* getData()
	{
		return data;
	}

	const uint8_t* getData() const
	{
		return data;
	}

	size_t getLength() const
	{
		return message_length;
	}
};

#endif
