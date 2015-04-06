## Copyright 2014 HSA Foundation Inc.  All Rights Reserved.
##
## HSAF is granting you permission to use this software and documentation (if
## any) (collectively, the "Materials") pursuant to the terms and conditions
## of the Software License Agreement included with the Materials.  If you do
## not have a copy of the Software License Agreement, contact the  HSA Foundation for a copy.
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in the
##    documentation and/or other materials provided with the distribution
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
## FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
## CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
## FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.

LFLAGS= -Wl,--unresolved-symbols=ignore-in-shared-libs

CC := gcc -m32

C_FILES := $(wildcard *.c)

OBJ_FILES := $(notdir $(C_FILES:.c=.o))

all: vector_copy

vector_copy: $(OBJ_FILES)
	$(CC) $(LFLAGS) $(OBJ_FILES) -L../../../lib/.libs -lm2s-hsa -o vector_copy

%.o: %.c
	$(CC) -c -I../../../runtime/include -o $@ $< -std=c99

clean:
	rm -rf *.o vector_copy
