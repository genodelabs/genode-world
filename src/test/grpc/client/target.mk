GRPC_DIR           := $(call select_from_ports,protobuf_grpc)/src/lib/grpc
PROTO_DIR          := $(GRPC_DIR)/examples/protos
TARGET             := grpc_client

LIBS               := posix
LIBS               += protobuf
LIBS               += stdcxx
LIBS               += grpc
LIBS               += libc_pipe

CC_CXX_WARN_STRICT :=

PROTOC             := /usr/local/genode/protobuf_grpc/current/bin/protoc
GRPC_PLUGIN        := /usr/local/genode/protobuf_grpc/current/bin/grpc_cpp_plugin

SRC_CC             := greeter_client.cc
SRC_CC             += helloworld.pb.cc
SRC_CC             += helloworld.grpc.pb.cc

vpath helloworld.proto   $(PROTO_DIR)

$(SRC_CC): helloworld.grpc.pb.h

helloworld.pb.h: helloworld.proto
	$(VERBOSE)$(PROTOC) --proto_path=$(PROTO_DIR) \
	                    --cpp_out=. \
	                    $<

helloworld.grpc.pb.h: helloworld.proto helloworld.pb.h
	$(VERBOSE)$(PROTOC) --plugin=protoc-gen-grpc=$(GRPC_PLUGIN) \
	                    --proto_path=$(PROTO_DIR) \
	                    --grpc_out=. \
	                    $<
