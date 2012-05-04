#!/usr/bin/python

import os

text_file = open("GL/gl.h", "r")
count = 1000
prefix='#define OPENGL_FUNC_'
for line in text_file:
	if line != "\n":
		item = line.split()
		if len(item) != 0:
			if item[0] == "GLAPI":
				line = line.replace('GLAPI ','')
				line = line.replace('GLAPIENTRY ','')
				line = line.replace('APIENTRY ','*')
				line = line.replace(';','')

				while ')' not in line:
					next_line = next(text_file)
					line = line + next_line

				line = line.replace(';','')
				# print line
				pos1 = line.index('(')

				if ')' in line:
					pos2 = line.index(')')
				
				args = line[pos1+1:pos2]
				# print args

				item = args.split(',')
				item_len = len(item)
				posgl = line.index('gl')
				posgln = line.index('(')
				func = line[posgl:posgln]
				print "#define OPENGL_FUNC_%s\t%d" %(func,count)
				count = count + 1

		
text_file.close()
