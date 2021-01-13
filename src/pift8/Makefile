CXXFLAGS ?= -std=c++14 -I.
LDFLAGS = -lm

pift8: pift8.o 
	$(CXX) $(LDFLAGS) -o $@ $^ -lrpitx -lft8

.PHONY: run_tests

run_tests: test
	@./test

test:  test.o
	$(CXX) $(LDFLAGS) -o $@ $^ -lft8
clean :
	rm pift8
