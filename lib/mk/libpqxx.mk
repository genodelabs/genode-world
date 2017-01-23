include $(REP_DIR)/lib/import/import-libpqxx.mk

LIBPQXX_PORT_DIR := $(call select_from_ports,libpqxx)
LIBPQXX_SRC_DIR  := $(LIBPQXX_PORT_DIR)/src/lib/libpqxx

# files 
SRC_CC += binarystring.cpp\
	connection.cpp\
	connection_base.cpp\
	cursor.cpp\
	dbtransaction.cpp\
	errorhandler.cpp\
	except.cpp\
	field.cpp\
	largeobject.cpp\
	nontransaction.cpp\
	notification.cpp\
	notify-listen.cpp\
	pipeline.cpp\
	prepared_statement.cpp\
	result.cpp\
	robusttransaction.cpp\
	statement_parameters.cpp\
	strconv.cpp\
	subtransaction.cpp\
	tablereader.cpp\
	tablestream.cpp\
	tablewriter.cpp\
	transaction.cpp\
	transaction_base.cpp\
	tuple.cpp\
	util.cpp

vpath % $(LIBPQXX_SRC_DIR)/src

LIBS += libc stdcxx libpq

SHARED_LIB = yes

