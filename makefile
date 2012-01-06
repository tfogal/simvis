SILO_ROOT=/home/tfogal/dev/visit/deps/visit/silo/4.7.2/a
SILO_CF=-I$(SILO_ROOT)/include
SILO_LD=-L$(SILO_ROOT)/lib -lsiloh5
UVF_CF=-I$(HOME)/dev/imagevis3d/Tuvok
UVF_LDFLAGS=$(HOME)/dev/imagevis3d/Tuvok/Build/libTuvok.a \
  $(HOME)/dev/imagevis3d/Tuvok/IO/expressions/libtuvokexpr.a
UVF_LD_REQS=-lz -lGLU -lGL -lQtOpenGL -lQtCore
H5_ROOT=/home/tfogal/dev/visit/deps/visit/hdf5/1.8.4/a
H5_LD=-L$(H5_ROOT)/lib -lhdf5
CXXFLAGS=-D_GLIBCXX_DEBUG -g -Wextra -Wall $(SILO_CF) $(UVF_CF)

OBJ=raw.o uvf.o

all: $(OBJ) ascii-to-uvf

bovconv: to-bov.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(SILO_LD) $(H5_LD)

ascii-to-uvf: uvf.o raw.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(UVF_LDFLAGS) $(UVF_LD_REQS)

clean:
	rm -f bovconv ascii-to-uvf $(OBJ)
