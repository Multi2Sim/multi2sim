EXE = vectorAdd
CPPS = $(wildcard *.cpp)
ALLCUS = $(wildcard *.cu)
INCUS = $(wildcard *_kernel*.cu)
CUS = $(filter-out $(INCUS),$(ALLCUS))
CUBINS = $(patsubst %.cu,%.cubin,$(CUS))
OBJDIR = obj
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.cpp.o,$(CPPS)) $(patsubst %.cu,$(OBJDIR)/%.cu.o,$(CUS))

CXX = g++
NVCC = nvcc
M2S = /home/zchen22/multi2sim/src/m2s
CXXFLAGS = 
NVCCFLAGS = -arch sm_20
SDKROOT = /home/zchen22/NVIDIA_GPU_Computing_SDK
INCS = -I. \
	-I/usr/local/cuda/include \
	-I$(SDKROOT)/shared/inc \
	-I$(SDKROOT)/C/common/inc
LIBS = -L/home/zchen22/multi2sim/tools/libm2s-cuda -lm2s-cuda \
	-L$(SDKROOT)/shared/lib -lshrutil_i386 \
	-L$(SDKROOT)/C/lib -lcutil_i386

$(EXE): makeobjdir $(OBJS) $(CUBINS)
	$(NVCC) -o $@ $(NVCCFLAGS) $(OBJS) $(LIBS)

makeobjdir:
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.cpp.o: %.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(INCS) -c $<

$(OBJDIR)/%.cu.o: %.cu
	$(NVCC) -o $@ $(NVCCFLAGS) $(INCS) -c $<

%.cubin: %.cu
	$(NVCC) -o $@ $(NVCCFLAGS) $(INCS) -cubin $<

run:
	$(M2S) $(EXE) --noprompt

clean:
	rm -fr *.csv *.txt $(EXE) *.sass *.cubin *.ptx $(OBJDIR)

