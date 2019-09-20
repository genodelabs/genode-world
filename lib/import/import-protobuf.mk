PROTOBUF_PORT_DIR := $(call select_from_ports,protobuf_grpc)

INC_DIR           += $(PROTOBUF_PORT_DIR)/include

PROTO_FILES_DIR   := $(PROTOBUF_PORT_DIR)/proto/protobuf/src
