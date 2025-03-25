all: speedtest

speedtest: speedtest1.o
	$(CC) $(LDFLAGS) $(LDLIBS) $< -o $@

speedtest1.o: speedtest1.c
	$(CC) -DSQLITE_ENABLE_RTREE $(CFLAGS) -c $< -o $@
