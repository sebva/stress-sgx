/*
 * Copyright (C) 2013-2018 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * This code is a complete clean re-write of the stress tool by
 * Colin Ian King <colin.king@canonical.com> and attempts to be
 * backwardly compatible with the stress tool by Amos Waterland
 * <apw@rossby.metr.ou.edu> but has more stress tests and more
 * functionality.
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/sctp.h>

/* The following functions from libsctp are used by stress-ng */

static void *sctp_funcs[] = {
	(void *)sctp_sendmsg,
	(void *)sctp_recvmsg,
};

#if !defined(SOL_SCTP)
#error no SOL_SCTP
#endif

#if !defined(IPPROTO_SCTP)
#error no IPPROTO_SCTP
#endif

int main(void)
{
	return 0;
}
