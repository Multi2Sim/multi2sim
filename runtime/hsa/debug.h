/*
 *  Multi2Sim
 *  Copyright (C) 2012  Yifan Sun (yifansun@coe.neu.edu)
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

#ifndef RUNTIME_HSA_DEBUG_H
#define RUNTIME_HSA_DEBUG_H

/* debug messages */
void m2s_hsa_warning(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void m2s_hsa_fatal(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void m2s_hsa_panic(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

#define warning m2s_hsa_warning
#define fatal m2s_hsa_fatal
#define panic m2s_hsa_panic

#endif /* RUNTIME_HSA_DEBUG_H_ */
