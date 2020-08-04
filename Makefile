SRCDIR=./src/
BINDIR=./bin/
CC=clang
CXX=clang++
LD=clang++
CFLAGS=-Wall -fPIC -I./include
CXXFLAGS=-std=c++17 -Wall -fPIC -I./include -Wno-overloaded-virtual
LD_LIBS=-lpthread -pthread
LD_LIBS=-lpthread -pthread
DBG_FLAGS=-g -I./libspatialaudio/build/Debug/include
OPT_FLAGS=-O3 -I./libspatialaudio/build/Release/include
HPP_FILES ?= $(shell find -L . -name '*.hpp')

SRCFILES=audio.cpp sound.cpp
OBJFILES=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRCFILES)))

DBG_SO_NAME=plugin.dbg.so
OPT_SO_NAME=plugin.opt.so

.PHONY: clean deepclean

$(DBG_SO_NAME): CFLAGS += $(DBG_FLAGS)
$(DBG_SO_NAME): CXXFLAGS += $(DBG_FLAGS)
$(DBG_SO_NAME): LIBSPATIALAUDIO_BUILD_TYPE=Debug
$(DBG_SO_NAME): $(OBJFILES) audio_component.o libspatialaudio/build/Debug/lib/libspatialaudio.a $(HPP_FILES)
	$(LD) $(CXXFLAGS) $(DBG_FLAGS) $(OBJFILES) audio_component.o libspatialaudio/build/Debug/lib/libspatialaudio.a -shared -o $@ $(LD_LIBS)

$(OPT_SO_NAME): CFLAGS += $(OPT_FLAGS)
$(OPT_SO_NAME): CXXFLAGS += $(OPT_FLAGS)
$(OPT_SO_NAME): LIBSPATIALAUDIO_BUILD_TYPE=Release
$(OPT_SO_NAME): $(OBJFILES) audio_component.o libspatialaudio/build/Release/lib/libspatialaudio.a $(HPP_FILES)
	$(LD) $(CXXFLAGS) $(OPT_FLAGS) $(OBJFILES) audio_component.o libspatialaudio/build/Release/lib/libspatialaudio.a -shared -o $@ $(LD_LIBS)

solo.dbg: CFLAGS += $(DBG_FLAGS)
solo.dbg: CXXFLAGS += $(DBG_FLAGS)
solo.dbg: LIBSPATIALAUDIO_BUILD_TYPE=Debug
solo.dbg: $(OBJFILES) main.o libspatialaudio/build/Debug/lib/libspatialaudio.a
	$(LD) $(DBG_FLAGS) $^ -o $@ $(LD_LIBS)

solo.opt: CFLAGS += $(OPT_FLAGS)
solo.opt: CXXFLAGS += $(OPT_FLAGS)
solo.opt: LIBSPATIALAUDIO_BUILD_TYPE=Release
solo.opt: $(OBJFILES) main.o libspatialaudio/build/Release/lib/libspatialaudio.a
	$(LD) $(OPT_FLAGS) $^ -o $@ $(LD_LIBS)

%.o: src/%.cpp libspatialaudio/build
	$(CXX) $(CXXFLAGS) $< -c -o $@

%.o: src/%.c libspatialaudio/build
	$(CC) $(CFLAGS) $< -c -o $@

libspatialaudio/build/Debug/lib/libspatialaudio.a: libspatialaudio/build
libspatialaudio/build/Release/lib/libspatialaudio.a: libspatialaudio/build

libspatialaudio/build:
	mkdir -p libspatialaudio/build/$(LIBSPATIALAUDIO_BUILD_TYPE)
	cd libspatialaudio/build; \
	cmake -DCMAKE_INSTALL_PREFIX=$(LIBSPATIALAUDIO_BUILD_TYPE) \
	      -DCMAKE_BUILD_TYPE=$(LIBSPATIALAUDIO_BUILD_TYPE) ..
	$(MAKE) -C libspatialaudio/build
	$(MAKE) -C libspatialaudio/build install

clean:
	rm -rf audio *.o *.so solo.dbg solo.opt

deepclean: clean
	rm -rf libspatialaudio/build
