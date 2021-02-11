# Gridware Serdes
C++ Serializer-Deserializer for nanopd-generated objects  

## Usage

    #include <Serializer.h>
    ...
	const gridware_DeviceResponse message={.verified=true};
    SerializerBuffer<decltype(message)> buffer(message);
	/* Serialized data now available in buffer.getData() and buffer.getLength() */
	...
	decltype(message) deserialized_message;
	buffer.deserialize(deserialized_message);
	assert(deserialized_message.verified == message.verified);

## Dependencies 
`python3-protobuf`  
`protobuf-compiler`  
`catch-devel` (for unit tests)

## Running simulation
`make`  
`./sim`  

## Running unit tests
`make test`
