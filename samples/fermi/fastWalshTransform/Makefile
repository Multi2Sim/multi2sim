# files
ELF = fastWalshTransform
CPPS = $(wildcard *.cpp)
ALLCUS = $(wildcard *.cu)
INCUS = $(wildcard *_kernel*.cu)
CUS = $(filter-out $(INCUS),$(ALLCUS))
CUBINS = $(patsubst %.cu,%.cubin,$(CUS))
OBJDIR = obj
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.cpp.o,$(CPPS)) $(patsubst %.cu,$(OBJDIR)/%.cu.o,$(CUS))
CUCPPS = $(patsubst %.cu,$(OBJDIR)/%.cu.cpp,$(CUS))

M2S_ROOT = ../../..
SDK_ROOT = /home/zchen22/CUDA_SDK_5_0

# compilers
CXX = g++
NVCC = nvcc
CXXFLAGS = -Wall -g -m32
NVCCFLAGS = -arch sm_20 -m 32
INCS = -I. -I$(SDK_ROOT)/common/inc
LIBS = -L$(M2S_ROOT)/lib/.libs -static -lm2s-cuda

$(ELF): makeobjdir $(OBJS) $(CUBINS) $(CUCPPS)
	$(CXX) -o $@ $(CXXFLAGS) $(OBJS) $(LIBS)

makeobjdir:
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.cpp.o: %.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(INCS) -c $<

$(OBJDIR)/%.cu.o: %.cu
	$(NVCC) -o $@ $(NVCCFLAGS) $(INCS) -c $<

%.cubin: %.cu
	$(NVCC) -o $@ $(NVCCFLAGS) $(INCS) -cubin $<

$(OBJDIR)/%.cu.cpp: %.cu
	$(NVCC) -o $@ $(NVCCFLAGS) $(INCS) -cuda $<

clean:
	rm -fr $(ELF) *.cubin $(OBJDIR)

