/*
 * nonet - execute a command without network access
 * Copyright (C) 2016 Israel G. Lugo
 *
 * This file is part of nonet.
 *
 * nonet is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * nonet is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with nonet. If not, see <http://www.gnu.org/licenses/>.
 *
 * For suggestions, feedback or bug reports: israel.lugo@lugosys.com
 */


/* uid.h - user handling module header */


#ifndef _UID_H
#define _UID_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif


#include <stdbool.h>
#include <pwd.h>

void set_user(uid_t uid);
void deal_with_uid(bool allow_root, bool user_specified, uid_t uid);
int get_user(uid_t *p_uid, const char *username);


#endif  /* _UID_H */
