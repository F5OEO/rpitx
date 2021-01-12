CXXFLAGS ?= -std=c++14 -I.
LDFLAGS = -lm

gen_ft8: gen_ft8.o common/wave.o
	$(CXX) $(LDFLAGS) -o $@ $^ -lpift8

pift8: pift8.o common/wave.o
	$(CXX) $(LDFLAGS) -o $@ $^ -lrpitx -lpift8

.PHONY: run_tests

run_tests: test
	@./test

test:  test.o
	$(CXX) $(LDFLAGS) -o $@ $^ -lft8

