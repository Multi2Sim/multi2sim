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

#ifndef ARCH_EVERGREEN_TIMING_FAULTS_H
#define ARCH_EVERGREEN_TIMING_FAULTS_H


/*
 * GPU-REL
 */

#define evg_faults_debug(...) debug(evg_faults_debug_category, __VA_ARGS__)
extern int evg_faults_debug_category;

extern char *evg_faults_debug_file_name;
extern char *evg_faults_file_name;

void evg_faults_init(void);
void evg_faults_done(void);

void evg_faults_insert(void);


#endif

