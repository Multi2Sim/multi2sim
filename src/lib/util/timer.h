/*
 *  Libstruct
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

#ifndef LIB_UTIL_TIMER_H
#define LIB_UTIL_TIMER_H

enum m2s_timer_state_t
{
	m2s_timer_state_invalid,
	m2s_timer_state_stopped,
	m2s_timer_state_running
};

struct m2s_timer_t;

struct m2s_timer_t *m2s_timer_create(char *name);
void m2s_timer_free(struct m2s_timer_t *timer);

void m2s_timer_start(struct m2s_timer_t *timer);
void m2s_timer_stop(struct m2s_timer_t *timer);
void m2s_timer_reset(struct m2s_timer_t *timer);

long long m2s_timer_get_value(struct m2s_timer_t *timer);
enum m2s_timer_state_t m2s_timer_get_state(struct m2s_timer_t *timer);
char *m2s_timer_get_name(struct m2s_timer_t *timer);



#endif
