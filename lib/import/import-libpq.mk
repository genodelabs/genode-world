INC_DIR += $(call select_from_ports,pgsql)/src/lib/postgres/src/include
INC_DIR += $(call select_from_ports,pgsql)/src/lib/postgres/src/include/libpq
INC_DIR += $(call select_from_ports,pgsql)/src/lib/postgres/src/interfaces/libpq
INC_DIR += $(call select_from_ports,pgsql)/src/lib/postgres/src/port
INC_DIR +=$(REP_DIR)/include/pq
