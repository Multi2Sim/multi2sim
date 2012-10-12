/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef MEM_SYSTEM_COMMAND_H
#define MEM_SYSTEM_COMMAND_H

/*
 * Event to handle memory hierarchy commands that initialize memory modules to a
 * certain state. These commands are used only for debugging purposes.
 */

extern int EV_MEM_SYSTEM_COMMAND;
extern int EV_MEM_SYSTEM_END_COMMAND;

void mem_system_command_handler(int event, void *data);
void mem_system_end_command_handler(int event, void *data);


#endif

