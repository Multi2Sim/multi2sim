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

#ifndef TIMER_H
#define TIMER_H

enum timer_state_t
{
	timer_state_invalid,
	timer_state_stopped,
	timer_state_running
};

struct timer_t;

struct timer_t *timer_create(char *name);
void timer_free(struct timer_t *timer);

void timer_start(struct timer_t *timer);
void timer_stop(struct timer_t *timer);
void timer_reset(struct timer_t *timer);

long long timer_get_value(struct timer_t *timer);
enum timer_state_t timer_get_state(struct timer_t *timer);
char *timer_get_name(struct timer_t *timer);



#endif
