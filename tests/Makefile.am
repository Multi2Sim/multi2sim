AM_CPPFLAGS = -isystem \
	      $(top_srcdir)/src/lib/gtest/include \
	      $(GTEST_CPPFLAGS) \
	      @M2S_INCLUDES@
AM_CXXFLAGS = $(GTEST_CXXFLAGS)
AM_LDFLAGS = $(top_builddir)/src/lib/gtest/lib/libgtest_main.la \
	     $(top_builddir)/src/lib/gtest/lib/libgtest.la \
	     $(GTEST_LIBS) $(GTEST_LDFLAGS) -pthread


TESTS = \
	src_arch_x86_timing_test \
	\
	src_arch_southern_islands_emu_test \
	\
	src_arch_southern_islands_timing_test \
	\
	src_lib_esim_test \
	\
	src_memory_test \
	\
	src_network_test \
	\
	src_dram_test

check_PROGRAMS = \
	src_arch_x86_timing_test \
	\
	src_arch_southern_islands_emu_test \
	\
	src_arch_southern_islands_timing_test \
	\
	src_lib_esim_test \
	\
	src_memory_test \
	\
	src_network_test \
	\
	src_dram_test


src_lib_esim_test_LDADD = \
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/lib/cpp/libcpp.a

src_lib_esim_test_SOURCES = \
	src/lib/esim/TestEngine.cc 

src_network_test_LDADD = \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/lib/cpp/libcpp.a \
	$(am__append_2) -lz

src_network_test_SOURCES = \
	src/network/TestNetworkConfig.cc \
	src/network/TestNetworkEvents.cc

src_dram_test_LDADD = \
	$(top_builddir)/src/dram/libdram.a \
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/lib/cpp/libcpp.a

src_dram_test_SOURCES = \
	src/dram/TestDramConfig.cc \
	src/dram/TestDramEvents.cc

src_arch_x86_timing_test_LDADD = \
	$(top_builddir)/src/arch/x86/timing/libtiming.a \
	$(top_builddir)/src/arch/x86/emulator/libemulator.a \
	$(top_builddir)/src/arch/x86/disassembler/libdisassembler.a \
	$(top_builddir)/src/arch/common/libcommon.a \
	$(top_builddir)/src/memory/libmemory.a \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/lib/cpp/libcpp.a \
	-lz

src_arch_x86_timing_test_SOURCES = \
	src/arch/x86/timing/ObjectPool.h \
	src/arch/x86/timing/ObjectPool.cc \
	src/arch/x86/timing/TestBranchPredictor.cc \
	src/arch/x86/timing/TestTraceCache.cc \
	src/arch/x86/timing/TestAlu.cc \
	src/arch/x86/timing/TestRegisterFile.cc \
	src/arch/x86/timing/TestFetch.cc
	
	
	
src_arch_southern_islands_emu_test_LDADD = \
	$(top_builddir)/src/arch/southern-islands/emulator/libemulator.a \
	$(top_builddir)/src/arch/southern-islands/disassembler/libdisassembler.a \
	$(top_builddir)/src/arch/common/libcommon.a \
	$(top_builddir)/src/memory/libmemory.a \
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/lib/cpp/libcpp.a

src_arch_southern_islands_emu_test_SOURCES = \
	src/arch/southern-islands/emu/ObjectPool.cc \
	src/arch/southern-islands/emu/ObjectPool.h \
	src/arch/southern-islands/emu/TestISAVOP2.cc \
	src/arch/southern-islands/emu/TestISASOP2.cc 

src_arch_southern_islands_timing_test_LDADD = \
	$(top_builddir)/src/arch/southern-islands/timing/libtiming.a \
	$(top_builddir)/src/arch/southern-islands/emulator/libemulator.a \
	$(top_builddir)/src/arch/southern-islands/disassembler/libdisassembler.a \
	$(top_builddir)/src/arch/common/libcommon.a \
	$(top_builddir)/src/memory/libmemory.a \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/lib/cpp/libcpp.a \
	-lz
	
src_arch_southern_islands_timing_test_SOURCES = \
	src/arch/southern-islands/timing/TestTiming.cc 
	

src_memory_test_LDADD = \
	$(top_builddir)/src/arch/x86/timing/libtiming.a \
	$(top_builddir)/src/arch/x86/emulator/libemulator.a \
	$(top_builddir)/src/arch/x86/disassembler/libdisassembler.a \
	$(top_builddir)/src/memory/libmemory.a \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/arch/common/libcommon.a \
	$(top_builddir)/src/lib/cpp/libcpp.a \
	$(am__append_2) -lz

src_memory_test_SOURCES = \
	src/memory/TestSystemConfig.cc \
	src/memory/TestSystemEvents.cc \
	src/memory/TestModule.cc

