all: test-smartcard

test-smartcard: main.o
	$(CC) $(LDFLAGS) $(LDLIBS) $< -o $@

main.o: main.cc
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@
