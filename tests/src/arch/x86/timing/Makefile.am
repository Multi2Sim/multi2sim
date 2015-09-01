AM_CPPFLAGS = -isystem \
	      $(top_srcdir)/src/lib/gtest/include \
	      $(GTEST_CPPFLAGS) \
	      @M2S_INCLUDES@
AM_CXXFLAGS = $(GTEST_CXXFLAGS)
AM_LDFLAGS = $(top_srcdir)/src/lib/gtest/lib/libgtest_main.la \
	     $(top_srcdir)/src/lib/gtest/lib/libgtest.la \
	     $(GTEST_LIBS) $(GTEST_LDFLAGS) -pthread
	     
TESTS = fetch_test

check_PROGRAMS = fetch_test

fetch_test_LDADD = \
	$(top_builddir)/src/arch/x86/timing/libtiming.a \
	$(top_builddir)/src/arch/x86/emulator/libemulator.a \
	$(top_builddir)/src/arch/x86/disassembler/libdisassembler.a \
	$(top_builddir)/src/arch/common/libcommon.a \
	$(top_builddir)/src/memory/libmemory.a \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/lib/cpp/libcpp.a \
	-lz
	
fetch_test_SOURCES = \
	TestFetch.cc