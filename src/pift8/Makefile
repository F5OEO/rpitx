CXXFLAGS = -std=c++14 -I.
LDFLAGS = -lm

gen_ft8: gen_ft8.o ft8/encode.o ft8/pack.o ft8/text.o ft8/pack_77.o ft8/encode_91.o common/wave.o
	$(CXX) $(LDFLAGS) -o $@ $^

pift8: pift8.o ft8/encode.o ft8/pack.o ft8/text.o ft8/pack_77.o ft8/encode_91.o common/wave.o ../librpitx/src/librpitx.a
	$(CXX) $(LDFLAGS) -o $@ $^

.PHONY: run_tests

run_tests: test
	@./test

test:  test.o ft8/encode.o ft8/pack.o ft8/text.o ft8/pack_77.o ft8/encode_91.o ft8/unpack.o
	$(CXX) $(LDFLAGS) -o $@ $^

