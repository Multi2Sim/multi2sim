AM_CPPFLAGS = -isystem $(top_srcdir)/src/lib/gtest/include $(GTEST_CPPFLAGS) \
	      @M2S_INCLUDES@
AM_CXXFLAGS = $(GTEST_CXXFLAGS)
AM_LDFLAGS = $(top_srcdir)/src/lib/gtest/lib/libgtest_main.la \
	     $(top_srcdir)/src/lib/gtest/lib/libgtest.la \
	     $(GTEST_LIBS) $(GTEST_LDFLAGS) -pthread


TESTS =
check_PROGRAMS = 


TESTS += hsa_asm_test
check_PROGRAMS += hsa_asm_test

hsa_asm_test_SOURCES = \
	TestBrigFile.cc \
	TestBrigSection.cc

hsa_asm_test_LDADD = \
	$(top_builddir)/src/arch/hsa/asm/libasm.a \
	$(top_builddir)/src/lib/cpp/libcpp.a

