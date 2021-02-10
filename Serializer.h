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
		-1;//reinterpret_cast<T>(-1);
}


// Buffer we load our message to/from
// Size is user-controllable, but has a 
// sensible default
template<class T>
class SerializerBuffer
{
	// Fixed size of our data
	static constexpr size_t data_size = _serializer_sizeOfType<typename std::remove_const<T>::type>();

	// Actual data
	uint8_t data[data_size];

	// This is filled in by the encoder function
	size_t message_length = 0;

	// Get specific PB fields object associated with type
	// Used for calling the pb_encode() and pb_decode() functions
	//
	// Usage: _serialize_fieldsOfType<T>()
	static constexpr decltype(gridware_DeviceResponse_fields) fieldsOfType()
	{
		return
			std::is_same<typename std::remove_const<T>::type, gridware_DeviceResponse>::value?
				gridware_DeviceResponse_fields:
			std::is_same<typename std::remove_const<T>::type, gridware_FirmwareImagePage>::value?
				gridware_FirmwareImagePage_fields:
			nullptr;
	}
public:

	SerializerBuffer(){};
	SerializerBuffer(const T& msg){
		static_assert(fieldsOfType() != nullptr);
		serialize(msg);
	};

	// Serialize a message into the buffer
	int serialize(const T& message)
	{
		pb_ostream_t stream = pb_ostream_from_buffer(data, data_size);

		const auto status = 
			pb_encode(&stream, fieldsOfType() ,&message);

		message_length = stream.bytes_written;

		if(!status)
		{
			message_length = 0;
			return -1;
		}
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

	// Return full buffer size
	size_t getDataSize() const
	{
		return data_size;
	}

	// Return length of message - only makes sense if already serialized
	size_t getLength() const
	{
		return message_length;
	}
	void setLength(size_t s)
	{
		message_length = s;
	}
};


#endif
