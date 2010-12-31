/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <options.h>
#include <m2skernel.h>


/* Options */
static uint32_t start_address = 0;
static uint32_t stop_address = 0;



static void sim_reg_options()
{
	opt_reg_uint32("-start_address", "address to start dump", &start_address);
	opt_reg_uint32("-stop_address", "address to stop dump", &stop_address);
}


static void sim_dump_section(void *buf, uint32_t addr, int size)
{
	uint32_t start, stop, offset;
	x86_inst_t inst;

	start = start_address > addr ? start_address - addr : 0;
	if (!stop_address || stop_address >= addr + size - 1)
		stop = size - 1;
	else
		stop = stop_address - addr;

	for (offset = start; offset <= stop; offset += inst.size) {
		x86_disasm(buf + offset, addr + offset, &inst);
		printf("%x ", addr + offset);
		if (inst.opcode == op_none) {
			printf("???\n");
			break;
		}
		x86_inst_dump(&inst, stdout);
		printf("\n");
	}
}


static void sim_dump(char *file)
{
	struct elf_file_t *elf;
	void *buf;
	uint32_t addr, size, flags;
	char *name;
	int i;

	/* Open file */
	elf = elf_open(file);
	if (!elf)
		fatal("%s: cannot open file", file);
	
	/* Sections */
	for (i = 0; i < elf_section_count(elf); i++) {
		elf_section_info(elf, i, &name, &addr, &size, &flags);
		if (!(flags & SHF_EXECINSTR))
			continue;

		/* Not in requested range */
		if (stop_address && stop_address < addr)
			continue;
		if (start_address >= addr + size)
			continue;

		/* Obtain buffer */
		buf = elf_section_read(elf, i);
		sim_dump_section(buf, addr, size);
		elf_free_buffer(buf);
	}
}


int main(int argc, char **argv)
{
	/* Initial information */
	fprintf(stderr, "\nMulti2Sim. Executable file dumper (m2s-objdump)\n");
	fprintf(stderr, "Last compilation: %s %s\n\n", __DATE__, __TIME__);

	/* Options */
	opt_init();
	sim_reg_options();
	opt_check_options(&argc, argv);
	opt_print_options(stderr);

	if (argc != 2) {
		fprintf(stderr, "syntax: m2s-objdump [<options>] <file>\n");
		exit(1);
	}

	disasm_init();
	sim_dump(argv[1]);
	disasm_done();

	opt_done();
	return 0;
}
