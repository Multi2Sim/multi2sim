# files
ELF = vectorAdd
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
CXXFLAGS = -Wall -m32
NVCCFLAGS = -arch sm_20 -m 32
INCS = -I.
LIBS = -L/usr/local/lib -static -lm2s-cuda

$(ELF): makeobjdir $(OBJS) $(CUBIN)
	$(CXX) -o $@ $(CXXFLAGS) $(OBJS) $(LIBS)

makeobjdir:
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.cpp.o: %.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(INCS) -c $<

$(OBJDIR)/%.cu.o: %.cu
	$(NVCC) -o $@ $(NVCCFLAGS) $(INCS) -c $<

%.cubin: %.cu
	$(NVCC) -o $@ $(NVCCFLAGS) $(INCS) -cubin $<

clean:
	rm -fr $(ELF) *.cubin $(OBJDIR)

