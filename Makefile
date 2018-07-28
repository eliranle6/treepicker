.PHONY: all
all:
	if [ ! -d build ]; then echo Please run 'make configure' first.; fi
	cd build && make

.PHONY: configure
configure: m4/ax_cxx_compile_stdcxx.m4
	autoreconf --install
	mkdir -p build
	cd build && ../configure

.PHONY: check
check:
	if [ ! -d build ]; then echo Please run 'make configure' first.; fi
	cd build && make check

.PHONY: clean
clean:
	if [ ! -d build ]; then echo Please run 'make configure' first.; fi
	cd build && make clean


m4/ax_cxx_compile_stdcxx.m4:
	cp scrips/$@ $@
