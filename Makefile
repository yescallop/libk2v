.PHONY: all format dev check clean test fuzz fuzz_parse fuzz_roundtrip
FUZZ_CXX ?= clang++
FUZZ_CFLAGS ?= -g -O1 -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer -Isrc/include -Wno-gnu-zero-variadic-macro-arguments
all:
	$(CC) -fPIC -shared -fstack-protector-all -fstack-clash-protection   -D_FORTIFY_SOURCE=3 -Wno-unused-result -O2 -std=gnu99 -Wno-gnu-zero-variadic-macro-arguments -o libk2v.so src/k2v.c -z noexecstack -z now
	strip libk2v.so
	$(CC) -std=gnu99 -c -o libk2v.o src/k2v.c
	ar -r libk2v.a libk2v.o
	rm libk2v.o
format:
	clang-format -i src/*.c
	clang-format -i src/include/*.h
	clang-format -i test/*.c
dev:
	$(CC) -std=gnu99 -fPIC -shared -ggdb -O0 -fno-omit-frame-pointer -z norelro -z execstack -Wno-gnu-zero-variadic-macro-arguments -fno-stack-protector -Wall -Wextra -pedantic -Wconversion -Wno-newline-eof -o libk2v.so src/k2v.c
	$(CC) -std=gnu99 -ggdb -O0 -fno-omit-frame-pointer -Wno-gnu-zero-variadic-macro-arguments -fno-stack-protector -Wall -Wextra -pedantic -Wconversion -Wno-newline-eof -c -o libk2v.o src/k2v.c
	ar -r libk2v.a libk2v.o
	rm libk2v.o
check:
	clang-tidy --checks=*,-clang-analyzer-security.insecureAPI.strcpy,-altera-unroll-loops,-cert-err33-c,-concurrency-mt-unsafe,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,-readability-function-cognitive-complexity,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers,-bugprone-easily-swappable-parameters,-cert-err34-c,-misc-include-cleaner,-readability-identifier-length,-bugprone-signal-handler,-cert-msc54-cpp,-cert-sig30-c,-altera-id-dependent-backward-branch,-cppcoreguidelines-avoid-non-const-global-variables src/k2v.c --
clean:
	rm k2sh *.so *.a||true
	rm testk2v||true
	rm -f fuzz/fuzz_parse fuzz/fuzz_roundtrip
test:
	$(CC) -std=gnu99 -fno-omit-frame-pointer -Wno-gnu-zero-variadic-macro-arguments -fno-stack-protector -Wall -Wextra -pedantic -Wconversion -Wno-newline-eof -fsanitize=address,undefined -g -O0 -Isrc/include test/test_all.c src/k2v.c -o test/test_all
	./test/test_all

fuzz: fuzz_parse fuzz_roundtrip

fuzz_parse: fuzz/fuzz_parse.c src/k2v.c
	mkdir -p fuzz
	$(FUZZ_CXX) $(FUZZ_CFLAGS) -x c src/k2v.c -x c fuzz/fuzz_parse.c -o fuzz/fuzz_parse

fuzz_roundtrip: fuzz/fuzz_roundtrip.c src/k2v.c
	mkdir -p fuzz
	$(FUZZ_CXX) $(FUZZ_CFLAGS) -x c src/k2v.c -x c fuzz/fuzz_roundtrip.c -o fuzz/fuzz_roundtrip
