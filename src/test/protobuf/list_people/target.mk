PROTOBUF_DIR := $(call select_from_ports,protobuf_grpc)/src/lib/protobuf

TARGET       := list_people

LIBS         := posix protobuf stdcxx

SRC_CC       += list_people.cc \
                addressbook.pb.cc

PROTOC       := /usr/local/genode/tool/current/bin/protoc

CC_CXX_WARN_STRICT =

vpath list_people.cc     $(PROTOBUF_DIR)/examples
vpath addressbook.proto  $(PROTOBUF_DIR)/examples

$(SRC_CC): addressbook.pb.h

addressbook.pb.h: addressbook.proto
	$(VERBOSE)$(PROTOC) --proto_path=$(PROTOBUF_DIR)/examples \
	                    --proto_path=$(PROTO_FILES_DIR) \
	                    --cpp_out=$(shell pwd) \
	                    $<
