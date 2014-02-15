/*
 *  Multi2Sim
 *  Copyright (C) 2013  Chris Barton (barton.ch@husky.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <string.h>

#include <llvm-c/Core.h>
#include "lib/mhandle/mhandle.h"

#include "format.h"
#include "Type.h"

char *extract_file_name(char *text)
{
	int i = 1;
	int j = 0;
	char buffer[100];
	char *ret_buffer;
	
	/*Copy file name to buffer*/
	while(text[i] != 34)
	{
		i++;
	}
	i++;
	while(text[i] != 34)
	{
		buffer[j] = text[i];
		i++;
		j++;
	}
	buffer[j] = '\00';
	ret_buffer = xstrdup(buffer);

	return ret_buffer;
}

/* This function takes cl2llvm_type and an empty string.  The string is
   then filled with the type of the cl2llvm_type */
void cl2llvm_type_to_string(struct cl2llvmTypeWrap *type, char *type_string)
{
	int sign;
	int i, j;
	int vec_size;
	int ptr_count;
	int array_count;
	int array_size[50];
	char type_string_cpy[50];
	LLVMTypeRef bit_type;

	bit_type = cl2llvmTypeWrapGetLlvmType(type);
	sign = cl2llvmTypeWrapGetSign(type);
	array_count = 0;
	ptr_count = 0;
	vec_size = 0;

	/* Get array information */
	if (LLVMGetTypeKind(bit_type) == LLVMArrayTypeKind)
	{
		for (array_count = 0; LLVMGetTypeKind(bit_type)
			== LLVMArrayTypeKind; array_count++)
		{
			array_size[array_count] = LLVMGetArrayLength(bit_type);
			bit_type = LLVMGetElementType(bit_type);
		}
	}
	/* Get pointer information */
	if (LLVMGetTypeKind(bit_type) == LLVMPointerTypeKind)
	{
		for (ptr_count = 0; LLVMGetTypeKind(bit_type) 
			== LLVMPointerTypeKind; ptr_count++)
		{
			bit_type = LLVMGetElementType(bit_type);
		}
	}
	/* Get vector information */
	if (LLVMGetTypeKind(bit_type) == LLVMVectorTypeKind)
	{
		vec_size = LLVMGetVectorSize(bit_type);
		bit_type = LLVMGetElementType(bit_type);
	}
	if (LLVMGetTypeKind(bit_type) == LLVMDoubleTypeKind)
		strcpy(type_string, "double");	
	else if (LLVMGetTypeKind(bit_type) == LLVMFloatTypeKind)
		strcpy(type_string, "float");
	else if (LLVMGetTypeKind(bit_type) == LLVMHalfTypeKind)
		strcpy(type_string, "half");
	else if (LLVMGetTypeKind(bit_type) == LLVMIntegerTypeKind)
	{
		if (bit_type == LLVMInt64Type())
		{
			if (sign)
				strcpy(type_string, "long long");
			else
				strcpy(type_string, "unsigned long long");
		}
		else if (bit_type == LLVMInt32Type())
		{
			if (sign)
				strcpy(type_string, "int");
			else
				strcpy(type_string, "uint");
		}
		else if (bit_type == LLVMInt16Type())
		{
			if (sign)
				strcpy(type_string, "short");
			else 
				strcpy(type_string, "ushort");
		}
		else if (bit_type == LLVMInt8Type())
		{
			if (sign)
				strcpy(type_string, "char");
			else
				strcpy(type_string, "uchar");
		}
		else if (bit_type == LLVMInt1Type())
			strcpy(type_string, "bool");
	}	
	if (vec_size)
	{
		i = 0;
		while (type_string[i] != '\00')
			i++;
		switch (vec_size)
		{
		case 2:
			type_string[i] = '2';
			break;
		case 3:
			type_string[i] = '3';
			break;
		case 4:
			type_string[i] = '4';
			break;
		case 8:
			type_string[i] = '8';
			break;
		case 16:
			type_string[i] = '1';
			type_string[++i] = '6';
			break;
		}
		i++;
		type_string[i] = '\00';
	}
	if (ptr_count)
	{
		i = 0;
		while (type_string[i] != '\00')
			i++;
		type_string[i++] = ' ';
		for (j = 0; j < ptr_count; j++)
			type_string[i++] = '*';
		type_string[i] = '\00';
	}

	if (array_count == 1)
	{
		i = 0;
		while (type_string[i] != '\00')
			i++;
		type_string[i] = '*';
		i++;
		type_string[i] = '\00';
	}
	else if (array_count)
	{
		i = 0;
		while (type_string[i] != '\00')
			i++;
		type_string[i++] = '(';
		type_string[i++] = '*';
		type_string[i++] = ')';
		type_string[i] = '\00';

		for (j = 1; j < array_count; j++)
		{
			strcpy(type_string_cpy, type_string);
			snprintf(type_string, sizeof(char) * 40,
				"%s[%d]", type_string_cpy, array_size[j]);
		}
	}
}
