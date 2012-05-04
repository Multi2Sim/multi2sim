#!/usr/bin/python

import os

text_file = open("GL/gl.h", "r")
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
				print line
				pos1 = line.index('(')

				if ')' in line:
					pos2 = line.index(')')
				
				args = line[pos1+1:pos2]
				# print args

				item = args.split(',')
				item_len = len(item)
				# print item
				print "{"
				print "\tunsigned int sys_args[%d];" %(item_len)
				for i in range(item_len):
					argsi = str(item[i])
					argss = argsi.split()
					# print argss
					if len(argss) >= 2:
						aa = str(argss[len(argss)-1])
						aa = aa.replace('*','')
						print "\tsys_args[%d] = (unsigned int) %s;" %(i,aa)
				posgl = line.index('gl')
				posgln = line.index('(')
				func = line[posgl:posgln]
				returntype = line[0:posgl]
				returntype = returntype.replace(' ','')
				if returntype != "void":
					print "\treturn (%s) syscall(SYS_CODE_OPENGL, OPENGL_FUNC_%s, sys_args);" %(returntype, func)
				else:
					print "\tsyscall(SYS_CODE_OPENGL, OPENGL_FUNC_%s, sys_args);" %(func)
				print "}"
				print "\n"

		
text_file.close()
