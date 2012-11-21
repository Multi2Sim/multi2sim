M2S_ROOT = ../..
M2S_LIB = $(M2S_ROOT)/src/lib

AR = ar
CC = gcc
DEBUG_CFLAGS = -Wall -Werror -O0 -g -DMHANDLE
CFLAGS = -fPIC -m32 -I./include
CFLAGS += $(DEBUG_CFLAGS)  ## Remove for no debugging
ASM = nasm
ASMFLAGS = -f elf32


LIB_NAME = libm2s-clrt

OBJS = \
	command-queue.o \
	context.o \
	debug.o \
	device.o \
	elf-format.o \
	elf-loader.o \
	event.o \
	kernel.o \
	list.o \
	m2s-clrt.o \
	mem.o \
	mhandle.o \
	mkfibers.o \
	object.o \
	platform.o \
	program.o \
	sampler.o \
	thread-list.o


all: $(LIB_NAME).so $(LIB_NAME).a


#
# Dynamic and static library targets
#

$(LIB_NAME).so: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -shared -o $(LIB_NAME).so

$(LIB_NAME).a: $(OBJS)
	$(AR) cru $(LIB_NAME).a $(OBJS)

clean:
	rm -f *.o *.so *.a

depend:
	makedepend -- $(CFLAGS) -- $(OBJS:.o=.c)

.c.o:
	$(CC) $(CFLAGS) -c $*.c -o $@

.s.o:
	$(ASM) $(ASMFLAGS) $*.s -o $@




# DO NOT DELETE

command-queue.o: /usr/include/unistd.h /usr/include/features.h
command-queue.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
command-queue.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
command-queue.o: /usr/include/gnu/stubs-64.h /usr/include/bits/posix_opt.h
command-queue.o: /usr/include/bits/environments.h /usr/include/bits/types.h
command-queue.o: /usr/include/bits/typesizes.h /usr/include/bits/confname.h
command-queue.o: /usr/include/getopt.h /usr/include/stdio.h
command-queue.o: /usr/include/libio.h /usr/include/_G_config.h
command-queue.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
command-queue.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
command-queue.o: /usr/include/xlocale.h /usr/include/assert.h m2s-clrt.h
command-queue.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
command-queue.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
command-queue.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
command-queue.o: /usr/include/sys/types.h /usr/include/time.h
command-queue.o: /usr/include/sys/select.h /usr/include/bits/select.h
command-queue.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
command-queue.o: /usr/include/sys/sysmacros.h
command-queue.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
command-queue.o: /usr/include/pthread.h /usr/include/sched.h
command-queue.o: /usr/include/bits/sched.h /usr/include/bits/setjmp.h
command-queue.o: /usr/include/elf.h /usr/include/stdint.h
command-queue.o: /usr/include/bits/wchar.h ./include/CL/cl.h
command-queue.o: ./include/CL/cl_platform.h clrt-object.h debug.h
command-queue.o: ../../src/arch/x86/emu/clrt.dat
context.o: /usr/include/unistd.h /usr/include/features.h
context.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
context.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
context.o: /usr/include/gnu/stubs-64.h /usr/include/bits/posix_opt.h
context.o: /usr/include/bits/environments.h /usr/include/bits/types.h
context.o: /usr/include/bits/typesizes.h /usr/include/bits/confname.h
context.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
context.o: /usr/include/_G_config.h /usr/include/wchar.h
context.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
context.o: m2s-clrt.h /usr/include/stdlib.h /usr/include/bits/waitflags.h
context.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
context.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
context.o: /usr/include/sys/types.h /usr/include/time.h
context.o: /usr/include/sys/select.h /usr/include/bits/select.h
context.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
context.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
context.o: /usr/include/alloca.h /usr/include/pthread.h /usr/include/sched.h
context.o: /usr/include/bits/sched.h /usr/include/bits/setjmp.h
context.o: /usr/include/elf.h /usr/include/stdint.h /usr/include/bits/wchar.h
context.o: ./include/CL/cl.h ./include/CL/cl_platform.h clrt-object.h debug.h
context.o: ../../src/arch/x86/emu/clrt.dat
debug.o: /usr/include/assert.h /usr/include/features.h
debug.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
debug.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
debug.o: /usr/include/gnu/stubs-64.h debug.h /usr/include/stdio.h
debug.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
debug.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
debug.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
debug.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
debug.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
debug.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
debug.o: /usr/include/sys/types.h /usr/include/time.h
debug.o: /usr/include/sys/select.h /usr/include/bits/select.h
debug.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
debug.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
debug.o: /usr/include/alloca.h mhandle.h /usr/include/string.h
debug.o: /usr/include/xlocale.h list.h
device.o: /usr/include/unistd.h /usr/include/features.h
device.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
device.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
device.o: /usr/include/gnu/stubs-64.h /usr/include/bits/posix_opt.h
device.o: /usr/include/bits/environments.h /usr/include/bits/types.h
device.o: /usr/include/bits/typesizes.h /usr/include/bits/confname.h
device.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
device.o: /usr/include/_G_config.h /usr/include/wchar.h
device.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
device.o: /usr/include/assert.h /usr/include/string.h /usr/include/xlocale.h
device.o: /usr/include/pthread.h /usr/include/endian.h
device.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
device.o: /usr/include/sched.h /usr/include/time.h /usr/include/bits/sched.h
device.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/setjmp.h
device.o: /usr/include/limits.h /usr/include/bits/posix1_lim.h
device.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
device.o: /usr/include/bits/posix2_lim.h m2s-clrt.h /usr/include/stdlib.h
device.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
device.o: /usr/include/sys/types.h /usr/include/sys/select.h
device.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
device.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
device.o: /usr/include/alloca.h /usr/include/elf.h /usr/include/stdint.h
device.o: /usr/include/bits/wchar.h ./include/CL/cl.h
device.o: ./include/CL/cl_platform.h clrt-object.h debug.h
device.o: ../../src/arch/x86/emu/clrt.dat
elf-format.o: /usr/include/assert.h /usr/include/features.h
elf-format.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
elf-format.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
elf-format.o: /usr/include/gnu/stubs-64.h /usr/include/sys/stat.h
elf-format.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
elf-format.o: /usr/include/time.h /usr/include/bits/stat.h debug.h
elf-format.o: /usr/include/stdio.h /usr/include/libio.h
elf-format.o: /usr/include/_G_config.h /usr/include/wchar.h
elf-format.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
elf-format.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
elf-format.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
elf-format.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
elf-format.o: /usr/include/sys/types.h /usr/include/sys/select.h
elf-format.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
elf-format.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
elf-format.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
elf-format.o: elf-format.h /usr/include/elf.h /usr/include/stdint.h
elf-format.o: /usr/include/bits/wchar.h list.h mhandle.h
elf-format.o: /usr/include/string.h /usr/include/xlocale.h
elf-loader.o: /usr/include/elf.h /usr/include/features.h
elf-loader.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
elf-loader.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
elf-loader.o: /usr/include/gnu/stubs-64.h /usr/include/stdint.h
elf-loader.o: /usr/include/bits/wchar.h /usr/include/stdlib.h
elf-loader.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
elf-loader.o: /usr/include/endian.h /usr/include/bits/endian.h
elf-loader.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
elf-loader.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
elf-loader.o: /usr/include/time.h /usr/include/sys/select.h
elf-loader.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
elf-loader.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
elf-loader.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
elf-loader.o: /usr/include/stdio.h /usr/include/libio.h
elf-loader.o: /usr/include/_G_config.h /usr/include/wchar.h
elf-loader.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
elf-loader.o: /usr/include/string.h /usr/include/xlocale.h
elf-loader.o: /usr/include/assert.h /usr/include/dlfcn.h
elf-loader.o: /usr/include/bits/dlfcn.h debug.h
event.o: /usr/include/unistd.h /usr/include/features.h
event.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
event.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
event.o: /usr/include/gnu/stubs-64.h /usr/include/bits/posix_opt.h
event.o: /usr/include/bits/environments.h /usr/include/bits/types.h
event.o: /usr/include/bits/typesizes.h /usr/include/bits/confname.h
event.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
event.o: /usr/include/_G_config.h /usr/include/wchar.h
event.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
event.o: /usr/include/sys/time.h /usr/include/time.h /usr/include/bits/time.h
event.o: /usr/include/sys/select.h /usr/include/bits/select.h
event.o: /usr/include/bits/sigset.h m2s-clrt.h /usr/include/stdlib.h
event.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
event.o: /usr/include/endian.h /usr/include/bits/endian.h
event.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
event.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
event.o: /usr/include/alloca.h /usr/include/pthread.h /usr/include/sched.h
event.o: /usr/include/bits/sched.h /usr/include/bits/setjmp.h
event.o: /usr/include/elf.h /usr/include/stdint.h /usr/include/bits/wchar.h
event.o: ./include/CL/cl.h ./include/CL/cl_platform.h clrt-object.h debug.h
event.o: ../../src/arch/x86/emu/clrt.dat
kernel.o: /usr/include/unistd.h /usr/include/features.h
kernel.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
kernel.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
kernel.o: /usr/include/gnu/stubs-64.h /usr/include/bits/posix_opt.h
kernel.o: /usr/include/bits/environments.h /usr/include/bits/types.h
kernel.o: /usr/include/bits/typesizes.h /usr/include/bits/confname.h
kernel.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
kernel.o: /usr/include/_G_config.h /usr/include/wchar.h
kernel.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
kernel.o: /usr/include/elf.h /usr/include/stdint.h /usr/include/bits/wchar.h
kernel.o: /usr/include/assert.h /usr/include/string.h /usr/include/xlocale.h
kernel.o: m2s-clrt.h /usr/include/stdlib.h /usr/include/bits/waitflags.h
kernel.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
kernel.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
kernel.o: /usr/include/sys/types.h /usr/include/time.h
kernel.o: /usr/include/sys/select.h /usr/include/bits/select.h
kernel.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
kernel.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
kernel.o: /usr/include/alloca.h /usr/include/pthread.h /usr/include/sched.h
kernel.o: /usr/include/bits/sched.h /usr/include/bits/setjmp.h
kernel.o: ./include/CL/cl.h ./include/CL/cl_platform.h clrt-object.h debug.h
kernel.o: ../../src/arch/x86/emu/clrt.dat
list.o: list.h
m2s-clrt.o: /usr/include/unistd.h /usr/include/features.h
m2s-clrt.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
m2s-clrt.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
m2s-clrt.o: /usr/include/gnu/stubs-64.h /usr/include/bits/posix_opt.h
m2s-clrt.o: /usr/include/bits/environments.h /usr/include/bits/types.h
m2s-clrt.o: /usr/include/bits/typesizes.h /usr/include/bits/confname.h
m2s-clrt.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
m2s-clrt.o: /usr/include/_G_config.h /usr/include/wchar.h
m2s-clrt.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
m2s-clrt.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
m2s-clrt.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
m2s-clrt.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
m2s-clrt.o: /usr/include/sys/types.h /usr/include/time.h
m2s-clrt.o: /usr/include/sys/select.h /usr/include/bits/select.h
m2s-clrt.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
m2s-clrt.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
m2s-clrt.o: /usr/include/alloca.h /usr/include/string.h
m2s-clrt.o: /usr/include/xlocale.h m2s-clrt.h /usr/include/pthread.h
m2s-clrt.o: /usr/include/sched.h /usr/include/bits/sched.h
m2s-clrt.o: /usr/include/bits/setjmp.h /usr/include/elf.h
m2s-clrt.o: /usr/include/stdint.h /usr/include/bits/wchar.h ./include/CL/cl.h
m2s-clrt.o: ./include/CL/cl_platform.h clrt-object.h debug.h
m2s-clrt.o: ../../src/arch/x86/emu/clrt.dat
mem.o: /usr/include/unistd.h /usr/include/features.h
mem.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
mem.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
mem.o: /usr/include/gnu/stubs-64.h /usr/include/bits/posix_opt.h
mem.o: /usr/include/bits/environments.h /usr/include/bits/types.h
mem.o: /usr/include/bits/typesizes.h /usr/include/bits/confname.h
mem.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
mem.o: /usr/include/_G_config.h /usr/include/wchar.h
mem.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
mem.o: /usr/include/string.h /usr/include/xlocale.h /usr/include/stdlib.h
mem.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
mem.o: /usr/include/endian.h /usr/include/bits/endian.h
mem.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
mem.o: /usr/include/time.h /usr/include/sys/select.h
mem.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
mem.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
mem.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
mem.o: /usr/include/assert.h m2s-clrt.h /usr/include/pthread.h
mem.o: /usr/include/sched.h /usr/include/bits/sched.h
mem.o: /usr/include/bits/setjmp.h /usr/include/elf.h /usr/include/stdint.h
mem.o: /usr/include/bits/wchar.h ./include/CL/cl.h ./include/CL/cl_platform.h
mem.o: clrt-object.h debug.h ../../src/arch/x86/emu/clrt.dat
mhandle.o: /usr/include/stdio.h /usr/include/features.h
mhandle.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
mhandle.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
mhandle.o: /usr/include/gnu/stubs-64.h /usr/include/bits/types.h
mhandle.o: /usr/include/bits/typesizes.h /usr/include/libio.h
mhandle.o: /usr/include/_G_config.h /usr/include/wchar.h
mhandle.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
mhandle.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
mhandle.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
mhandle.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
mhandle.o: /usr/include/sys/types.h /usr/include/time.h
mhandle.o: /usr/include/sys/select.h /usr/include/bits/select.h
mhandle.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
mhandle.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
mhandle.o: /usr/include/alloca.h /usr/include/string.h /usr/include/xlocale.h
mhandle.o: /usr/include/assert.h
mkfibers.o: /usr/include/assert.h /usr/include/features.h
mkfibers.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
mkfibers.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
mkfibers.o: /usr/include/gnu/stubs-64.h /usr/include/string.h
mkfibers.o: /usr/include/xlocale.h /usr/include/stdio.h
mkfibers.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
mkfibers.o: /usr/include/libio.h /usr/include/_G_config.h
mkfibers.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
mkfibers.o: /usr/include/bits/sys_errlist.h m2s-clrt.h /usr/include/unistd.h
mkfibers.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
mkfibers.o: /usr/include/bits/confname.h /usr/include/getopt.h
mkfibers.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
mkfibers.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
mkfibers.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
mkfibers.o: /usr/include/sys/types.h /usr/include/time.h
mkfibers.o: /usr/include/sys/select.h /usr/include/bits/select.h
mkfibers.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
mkfibers.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
mkfibers.o: /usr/include/alloca.h /usr/include/pthread.h /usr/include/sched.h
mkfibers.o: /usr/include/bits/sched.h /usr/include/bits/setjmp.h
mkfibers.o: /usr/include/elf.h /usr/include/stdint.h
mkfibers.o: /usr/include/bits/wchar.h ./include/CL/cl.h
mkfibers.o: ./include/CL/cl_platform.h clrt-object.h debug.h
mkfibers.o: ../../src/arch/x86/emu/clrt.dat
object.o: /usr/include/assert.h /usr/include/features.h
object.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
object.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
object.o: /usr/include/gnu/stubs-64.h /usr/include/string.h
object.o: /usr/include/xlocale.h m2s-clrt.h /usr/include/unistd.h
object.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
object.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
object.o: /usr/include/bits/confname.h /usr/include/getopt.h
object.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
object.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
object.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
object.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
object.o: /usr/include/endian.h /usr/include/bits/endian.h
object.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
object.o: /usr/include/time.h /usr/include/sys/select.h
object.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
object.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
object.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
object.o: /usr/include/pthread.h /usr/include/sched.h
object.o: /usr/include/bits/sched.h /usr/include/bits/setjmp.h
object.o: /usr/include/elf.h /usr/include/stdint.h /usr/include/bits/wchar.h
object.o: ./include/CL/cl.h ./include/CL/cl_platform.h clrt-object.h debug.h
object.o: ../../src/arch/x86/emu/clrt.dat thread-list.h
platform.o: /usr/include/unistd.h /usr/include/features.h
platform.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
platform.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
platform.o: /usr/include/gnu/stubs-64.h /usr/include/bits/posix_opt.h
platform.o: /usr/include/bits/environments.h /usr/include/bits/types.h
platform.o: /usr/include/bits/typesizes.h /usr/include/bits/confname.h
platform.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
platform.o: /usr/include/_G_config.h /usr/include/wchar.h
platform.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
platform.o: /usr/include/string.h /usr/include/xlocale.h m2s-clrt.h
platform.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
platform.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
platform.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
platform.o: /usr/include/sys/types.h /usr/include/time.h
platform.o: /usr/include/sys/select.h /usr/include/bits/select.h
platform.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
platform.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
platform.o: /usr/include/alloca.h /usr/include/pthread.h /usr/include/sched.h
platform.o: /usr/include/bits/sched.h /usr/include/bits/setjmp.h
platform.o: /usr/include/elf.h /usr/include/stdint.h
platform.o: /usr/include/bits/wchar.h ./include/CL/cl.h
platform.o: ./include/CL/cl_platform.h clrt-object.h debug.h
platform.o: ../../src/arch/x86/emu/clrt.dat
program.o: /usr/include/unistd.h /usr/include/features.h
program.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
program.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
program.o: /usr/include/gnu/stubs-64.h /usr/include/bits/posix_opt.h
program.o: /usr/include/bits/environments.h /usr/include/bits/types.h
program.o: /usr/include/bits/typesizes.h /usr/include/bits/confname.h
program.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
program.o: /usr/include/_G_config.h /usr/include/wchar.h
program.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
program.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
program.o: /usr/include/string.h /usr/include/xlocale.h /usr/include/dlfcn.h
program.o: /usr/include/bits/dlfcn.h m2s-clrt.h /usr/include/stdlib.h
program.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
program.o: /usr/include/endian.h /usr/include/bits/endian.h
program.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
program.o: /usr/include/time.h /usr/include/sys/select.h
program.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
program.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
program.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
program.o: /usr/include/pthread.h /usr/include/sched.h
program.o: /usr/include/bits/sched.h /usr/include/bits/setjmp.h
program.o: /usr/include/elf.h /usr/include/stdint.h /usr/include/bits/wchar.h
program.o: ./include/CL/cl.h ./include/CL/cl_platform.h clrt-object.h debug.h
program.o: ../../src/arch/x86/emu/clrt.dat
sampler.o: /usr/include/unistd.h /usr/include/features.h
sampler.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
sampler.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
sampler.o: /usr/include/gnu/stubs-64.h /usr/include/bits/posix_opt.h
sampler.o: /usr/include/bits/environments.h /usr/include/bits/types.h
sampler.o: /usr/include/bits/typesizes.h /usr/include/bits/confname.h
sampler.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
sampler.o: /usr/include/_G_config.h /usr/include/wchar.h
sampler.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
sampler.o: m2s-clrt.h /usr/include/stdlib.h /usr/include/bits/waitflags.h
sampler.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
sampler.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
sampler.o: /usr/include/sys/types.h /usr/include/time.h
sampler.o: /usr/include/sys/select.h /usr/include/bits/select.h
sampler.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
sampler.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
sampler.o: /usr/include/alloca.h /usr/include/pthread.h /usr/include/sched.h
sampler.o: /usr/include/bits/sched.h /usr/include/bits/setjmp.h
sampler.o: /usr/include/elf.h /usr/include/stdint.h /usr/include/bits/wchar.h
sampler.o: ./include/CL/cl.h ./include/CL/cl_platform.h clrt-object.h debug.h
sampler.o: ../../src/arch/x86/emu/clrt.dat
thread-list.o: /usr/include/stdlib.h /usr/include/features.h
thread-list.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
thread-list.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
thread-list.o: /usr/include/gnu/stubs-64.h /usr/include/bits/waitflags.h
thread-list.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
thread-list.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
thread-list.o: /usr/include/sys/types.h /usr/include/bits/types.h
thread-list.o: /usr/include/bits/typesizes.h /usr/include/time.h
thread-list.o: /usr/include/sys/select.h /usr/include/bits/select.h
thread-list.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
thread-list.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
thread-list.o: /usr/include/alloca.h /usr/include/stdio.h
thread-list.o: /usr/include/libio.h /usr/include/_G_config.h
thread-list.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
thread-list.o: /usr/include/bits/sys_errlist.h thread-list.h
thread-list.o: /usr/include/pthread.h /usr/include/sched.h
thread-list.o: /usr/include/bits/sched.h /usr/include/bits/setjmp.h
