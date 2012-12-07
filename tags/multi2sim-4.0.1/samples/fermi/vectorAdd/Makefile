# tool directories
M2SROOT = /home/zchen22/multi2sim
SDKROOT = /home/zchen22/NVIDIA_GPU_Computing_SDK

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
M2S = $(M2SROOT)/src/m2s
CXXFLAGS = -Wall
NVCCFLAGS = -arch sm_20
INCS = -I. \
	-I$(M2SROOT)/tools/libm2s-cuda/cuda_inc \
	-I$(SDKROOT)/shared/inc \
	-I$(SDKROOT)/C/common/inc
LIBS = -L$(M2SROOT)/tools/libm2s-cuda -Wl,-Bstatic -lm2s-cuda \
	-L$(SDKROOT)/shared/lib -Wl,-Bdynamic -lshrutil_i386 \
	-L$(SDKROOT)/C/lib -Wl,-Bdynamic -lcutil_i386

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

run:
	$(M2S) --frm-disasm $(CUBIN) > $(CUBIN).sass
	$(M2S) --frm-sim functional $(EXE) --noprompt > $(EXE).sim

clean:
	rm -fr *.sim *.csv *.txt $(EXE) *.sass *.cubin *.ptx $(OBJDIR)

