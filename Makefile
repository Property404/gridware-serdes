PROTOC=./nanopb/generator/protoc
PROTO_BASE_NAME=objects
all:
	$(PROTOC) --nanopb_out=. $(PROTO_BASE_NAME).proto

clean:
	rm -f *.pb.c *.pb.h
