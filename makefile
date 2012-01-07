UVF_ROOT=./imagevis3d/Tuvok
UVF_CF=-I$(UVF_ROOT)
UVF_LDFLAGS=$(UVF_ROOT)/Build/libTuvok.a \
  $(UVF_ROOT)/IO/expressions/libtuvokexpr.a
UVF_LD_REQS=-lz -lGLU -lGL -lQtOpenGL -lQtCore
CXXFLAGS=-g -Wextra -Wall -O3 $(UVF_CF)

OBJ=raw.o uvf.o

all: $(OBJ) ascii-to-uvf

ascii-to-uvf: uvf.o raw.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(UVF_LDFLAGS) $(UVF_LD_REQS)

clean:
	rm -f ascii-to-uvf $(OBJ)
