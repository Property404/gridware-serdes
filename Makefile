NANOPB_DIR=./nanopb/
PROTOC=$(NANOPB_DIR)/generator/protoc

COMMON_COMPILER_FLAGS = -Wall -Wextra -fmax-errors=1 -I$(NANOPB_DIR) -Ilibcrc/include
CXXFLAGS=$(COMMON_COMPILER_FLAGS) -std=gnu++11 
CFLAGS=$(COMMON_COMPILER_FLAGS) -std=c11 

PROTO_BASE_NAME=objects
COMMON_OBJECTS =  $(PROTO_BASE_NAME).pb.o $(NANOPB_DIR)/pb_encode.o\
		  $(NANOPB_DIR)/pb_decode.o $(NANOPB_DIR)/pb_common.o\
		  Socket.o Server.o libcrc/src/crc16.o
UNIT_TEST_OBJECTS = $(COMMON_OBJECTS) unit_test_main.o test_cases.o
SIM_OBJECTS = $(COMMON_OBJECTS) network_simulate.o launch_server.o\
			  launch_client.o

all: proto sim

# Simulator executable
sim: $(SIM_OBJECTS)
	$(CXX) $(CXXFLAGS) $(SIM_OBJECTS) -o sim -lpthread

# Compile protobuf file
proto:
	$(PROTOC) --nanopb_out=. $(PROTO_BASE_NAME).proto
$(PROTO_BASE_NAME).pb.c: proto

# Build and run unit tests
test: proto $(UNIT_TEST_OBJECTS) 
	$(CXX) $(CXXFLAGS) $(UNIT_TEST_OBJECTS) -o test -lpthread
	./test

clean:
	rm -f $(UNIT_TEST_OBJECTS)
	rm -f *.pb.c *.pb.h *.o
	rm -f test
