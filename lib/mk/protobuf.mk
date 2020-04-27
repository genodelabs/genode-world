include $(REP_DIR)/lib/import/import-protobuf.mk

PROTOBUF_SRC_DIR   := $(PROTOBUF_PORT_DIR)/src/lib/grpc/third_party/protobuf/src/google/protobuf

LIBS               := base
LIBS               += libc
LIBS               += zlib
LIBS               += stdcxx

SHARED_LIB         := yes

CC_CXX_WARN_STRICT :=
CC_OPT             += -DHAVE_PTHREAD=0 -Wno-sign-compare

INC_DIR            += $(PROTOBUF_PORT_DIR)/src/lib/grpc/third_party/protobuf/src

SRC_CC := \
	any.cc                                       \
	any.pb.cc                                    \
	any_lite.cc                                  \
	api.pb.cc                                    \
	arena.cc                                     \
	descriptor.cc                                \
	descriptor.pb.cc                             \
	descriptor_database.cc                       \
	duration.pb.cc                               \
	dynamic_message.cc                           \
	empty.pb.cc                                  \
	extension_set.cc                             \
	extension_set_heavy.cc                       \
	field_mask.pb.cc                             \
	generated_message_reflection.cc              \
	generated_message_table_driven.cc            \
	generated_message_util.cc                    \
	implicit_weak_message.cc                     \
	map_field.cc                                 \
	message.cc                                   \
	message_lite.cc                              \
	parse_context.cc                             \
	reflection_ops.cc                            \
	repeated_field.cc                            \
	service.cc                                   \
	source_context.pb.cc                         \
	struct.pb.cc                                 \
	text_format.cc                               \
	timestamp.pb.cc                              \
	type.pb.cc                                   \
	unknown_field_set.cc                         \
	wire_format_lite.cc                          \
	wire_format.cc                               \
	wrappers.pb.cc                               \
	stubs/bytestream.cc                          \
	stubs/common.cc                              \
	stubs/int128.cc                              \
	stubs/status.cc                              \
	stubs/statusor.cc                            \
	stubs/stringpiece.cc                         \
	stubs/stringprintf.cc                        \
	stubs/structurally_valid.cc                  \
	stubs/strutil.cc                             \
	stubs/substitute.cc                          \
	stubs/time.cc                                \
	io/coded_stream.cc                           \
	io/gzip_stream.cc                            \
	io/io_win32.cc                               \
	io/printer.cc                                \
	io/strtod.cc                                 \
	io/tokenizer.cc                              \
	io/zero_copy_stream.cc                       \
	io/zero_copy_stream_impl.cc                  \
	io/zero_copy_stream_impl_lite.cc             \
	util/delimited_message_util.cc               \
	util/field_comparator.cc                     \
	util/field_mask_util.cc                      \
	util/json_util.cc                            \
	util/message_differencer.cc                  \
	util/time_util.cc                            \
	util/type_resolver_util.cc                   \
	util/internal/datapiece.cc                   \
	util/internal/default_value_objectwriter.cc  \
	util/internal/error_listener.cc              \
	util/internal/field_mask_utility.cc          \
	util/internal/json_escaping.cc               \
	util/internal/json_objectwriter.cc           \
	util/internal/json_stream_parser.cc          \
	util/internal/object_writer.cc               \
	util/internal/proto_writer.cc                \
	util/internal/protostream_objectsource.cc    \
	util/internal/protostream_objectwriter.cc    \
	util/internal/type_info.cc                   \
	util/internal/utility.cc                     \

vpath %.cc $(PROTOBUF_SRC_DIR)
