# files
EXE = vectorAdd
CPPS = $(wildcard *.cpp)
ALLCUS = $(wildcard *.cu)
INCUS = $(wildcard *_kernel*.cu)
CU = $(filter-out $(INCUS),$(ALLCUS))
CUBIN = $(patsubst %.cu,%.cubin,$(CU))
OBJDIR = obj
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.cpp.o,$(CPPS)) $(patsubst %.cu,$(OBJDIR)/%.cu.o,$(CU))

# compilers
CXX = g++
NVCC = nvcc
M2SROOT = $(HOME)/multi2sim
M2S = $(M2SROOT)/src/m2s
CXXFLAGS = -Wall
NVCCFLAGS = -arch sm_20
INCS = -I.
LIBS = -L$(M2SROOT)/tools/libm2s-cuda -static -lm2s-cuda

$(EXE): makeobjdir $(OBJS) $(CUBIN)
	$(CXX) -o $@ $(OBJS) $(LIBS)

makeobjdir:
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.cpp.o: %.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(INCS) -c $<

$(OBJDIR)/%.cu.o: %.cu
	$(NVCC) -o $@ $(NVCCFLAGS) $(INCS) -c $<

%.cubin: %.cu
	$(NVCC) -o $@ $(NVCCFLAGS) $(INCS) -cubin $<

clean:
	rm -fr *.csv *.txt $(EXE) *.cubin *.ptx $(OBJDIR)

