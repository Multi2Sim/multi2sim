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

#include <sched.h>
#include <syscall.h>
#include <time.h>
#include <utime.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/times.h>

#include <mem-system.h>
#include <mhandle.h>
#include <arm-emu.h>




/*
 * System call error codes
 */

#define SIM_EPERM		1
#define SIM_ENOENT		2
#define SIM_ESRCH		3
#define SIM_EINTR		4
#define SIM_EIO			5
#define SIM_ENXIO		6
#define SIM_E2BIG		7
#define SIM_ENOEXEC		8
#define SIM_EBADF		9
#define SIM_ECHILD		10
#define SIM_EAGAIN		11
#define SIM_ENOMEM		12
#define SIM_EACCES		13
#define SIM_EFAULT		14
#define SIM_ENOTBLK		15
#define SIM_EBUSY		16
#define SIM_EEXIST		17
#define SIM_EXDEV		18
#define SIM_ENODEV		19
#define SIM_ENOTDIR		20
#define SIM_EISDIR		21
#define SIM_EINVAL		22
#define SIM_ENFILE		23
#define SIM_EMFILE		24
#define SIM_ENOTTY		25
#define SIM_ETXTBSY		26
#define SIM_EFBIG		27
#define SIM_ENOSPC		28
#define SIM_ESPIPE		29
#define SIM_EROFS		30
#define SIM_EMLINK		31
#define SIM_EPIPE		32
#define SIM_EDOM		33
#define SIM_ERANGE		34

#define SIM_ERRNO_MAX		34



void arm_sys_init(void)
{
	/* Host constants for 'errno' must match */
	M2S_HOST_GUEST_MATCH(EPERM, SIM_EPERM);
	M2S_HOST_GUEST_MATCH(ENOENT, SIM_ENOENT);
	M2S_HOST_GUEST_MATCH(ESRCH, SIM_ESRCH);
	M2S_HOST_GUEST_MATCH(EINTR, SIM_EINTR);
	M2S_HOST_GUEST_MATCH(EIO, SIM_EIO);
	M2S_HOST_GUEST_MATCH(ENXIO, SIM_ENXIO);
	M2S_HOST_GUEST_MATCH(E2BIG, SIM_E2BIG);
	M2S_HOST_GUEST_MATCH(ENOEXEC, SIM_ENOEXEC);
	M2S_HOST_GUEST_MATCH(EBADF, SIM_EBADF);
	M2S_HOST_GUEST_MATCH(ECHILD, SIM_ECHILD);
	M2S_HOST_GUEST_MATCH(EAGAIN, SIM_EAGAIN);
	M2S_HOST_GUEST_MATCH(ENOMEM, SIM_ENOMEM);
	M2S_HOST_GUEST_MATCH(EACCES, SIM_EACCES);
	M2S_HOST_GUEST_MATCH(EFAULT, SIM_EFAULT);
	M2S_HOST_GUEST_MATCH(ENOTBLK, SIM_ENOTBLK);
	M2S_HOST_GUEST_MATCH(EBUSY, SIM_EBUSY);
	M2S_HOST_GUEST_MATCH(EEXIST, SIM_EEXIST);
	M2S_HOST_GUEST_MATCH(EXDEV, SIM_EXDEV);
	M2S_HOST_GUEST_MATCH(ENODEV, SIM_ENODEV);
	M2S_HOST_GUEST_MATCH(ENOTDIR, SIM_ENOTDIR);
	M2S_HOST_GUEST_MATCH(EISDIR, SIM_EISDIR);
	M2S_HOST_GUEST_MATCH(EINVAL, SIM_EINVAL);
	M2S_HOST_GUEST_MATCH(ENFILE, SIM_ENFILE);
	M2S_HOST_GUEST_MATCH(EMFILE, SIM_EMFILE);
	M2S_HOST_GUEST_MATCH(ENOTTY, SIM_ENOTTY);
	M2S_HOST_GUEST_MATCH(ETXTBSY, SIM_ETXTBSY);
	M2S_HOST_GUEST_MATCH(EFBIG, SIM_EFBIG);
	M2S_HOST_GUEST_MATCH(ENOSPC, SIM_ENOSPC);
	M2S_HOST_GUEST_MATCH(ESPIPE, SIM_ESPIPE);
	M2S_HOST_GUEST_MATCH(EROFS, SIM_EROFS);
	M2S_HOST_GUEST_MATCH(EMLINK, SIM_EMLINK);
	M2S_HOST_GUEST_MATCH(EPIPE, SIM_EPIPE);
	M2S_HOST_GUEST_MATCH(EDOM, SIM_EDOM);
	M2S_HOST_GUEST_MATCH(ERANGE, SIM_ERANGE);
}

void arm_sys_done(void)
{
	/* Print summary
	if (debug_status(arm_sys_debug_category))
		arm_sys_dump(debug_file(arm_sys_debug_category));
*/}
