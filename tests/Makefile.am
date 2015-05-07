AM_CPPFLAGS = -isystem \
	      $(top_srcdir)/src/lib/gtest/include \
	      $(GTEST_CPPFLAGS) \
	      @M2S_INCLUDES@
AM_CXXFLAGS = $(GTEST_CXXFLAGS)
AM_LDFLAGS = $(top_srcdir)/src/lib/gtest/lib/libgtest_main.la \
	     $(top_srcdir)/src/lib/gtest/lib/libgtest.la \
	     $(GTEST_LIBS) $(GTEST_LDFLAGS) -pthread


TESTS = 
TESTS += \
	src_lib_esim_test \
	\
	src_network_test \
	\
	src_arch_hsa_asm_test 

check_PROGRAMS = \
	src_lib_esim_test \
	\
	src_network_test \
	\
	src_arch_hsa_asm_test 



src_lib_esim_test_LDADD = \
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/lib/cpp/libcpp.a

src_lib_esim_test_SOURCES = \
	src/lib/esim/TestEngine.cc 



src_network_test_LDADD = \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/lib/cpp/libcpp.a

src_network_test_SOURCES = \
	src/network/TestNetwork.cc \
	src/network/TestNodeFactory.cc



src_arch_hsa_asm_test_LDADD = \
	$(top_builddir)/src/arch/hsa/asm/libasm.a \
	$(top_builddir)/src/lib/cpp/libcpp.a

src_arch_hsa_asm_test_SOURCES = \
	src/arch/hsa/asm/TestBrigFile.cc \
	src/arch/hsa/asm/TestBrigSection.cc \
	src/arch/hsa/asm/TestBrigCodeEntry.cc \
	src/arch/hsa/asm/TestBrigImmed.cc
