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


#if HAVE_CONFIG_H
#  include <config.h>
#endif


/* get getresuid() and setresuid() */
#define _GNU_SOURCE
#include <unistd.h>

#include <sys/types.h>
#include <pwd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "msg.h"



/*
 * Change user in a safe way.
 *
 * Sets the effective, real and saved user id of the process to the
 * specified uid. Makes sure the uid's were all changed correctly and
 * aborts the program otherwise.
 */
void set_user(uid_t uid)
{
    uid_t ruid, euid, suid;
    int status;

    /* TODO: Deal with groups. See what sudo does for an example. */

    status = setresuid(uid, uid, uid);
    if (status != 0)
        die("unable to set UID");

    /* Be extra paranoid: make sure we really set both effective UID
     * and saved UID (happened before, bugs in kernel < 2.2.16) */
    status = getresuid(&ruid,&euid,&suid);
    if (status != 0)
        die("unable to check if UID was changed properly");

    if (ruid != uid || euid != uid || suid != uid)
        die("changed UID but it didn't change! (insanity, quitting for security)");
}


/*
 * Try to loose root privileges in any way possible.
 *
 * If the user specified a non-root uid, change to it. If the program was
 * run set-uid (real uid different from effective uid), change to the real
 * uid. If not actually running as root (e.g. using POSIX capabilities), or
 * running as root but allow_root is true, run as-is. Otherwise (running as
 * root and no real or specified uid to fallback to), error out.
 */
void deal_with_uid(bool allow_root, bool user_specified, uid_t uid)
{
    if (user_specified)
    {
        if (uid == 0 && !allow_root)
        {
            fprintf(stderr,
                    "%s: refusing to run program as root\n"
                    "%s: use --allow-root to force\n",
                    prog_name, prog_name);
            exit(2);
        }
        set_user(uid);
    }
    else
    {   /* no user specified: run as-is or squash to ruid if we're set-uid */
        uid_t ruid = getuid();
        uid_t euid = geteuid();

        if (ruid == 0 && !allow_root)
        {   /* running from real root (e.g. sudo/su) */
            fprintf(stderr,
                    "%s: refusing to run program as root\n"
                    "%s: use --user to specify another user, or --allow-root to force\n",
                    prog_name, prog_name);
            exit(2);
        }

        if (ruid != euid)
        {   /* we're probably running from a set-uid file */
            set_user(ruid);
        }
    }
}


/*
 * Get a UID from a user name or numeric user id.
 *
 * Sets the uid_t pointed to by p_uid, if the user is found. Returns zero in
 * case of success, 1 if the user was not found, or 2 if there's an error in
 * errno.
 */
int get_user(uid_t *p_uid, const char *username)
{
    struct passwd *passwd;
    char *endptr;
    long maybe_uid;
    int retval = 0;

    errno = 0;
    maybe_uid = strtol(username, &endptr, 10);

    if (endptr == username || *endptr != '\0')
    {   /* string wasn't (only) numeric; probably a user name */
        errno = 0;
        passwd = getpwnam(username);
    }
    else if (errno != 0)
    {   /* some kind of error (perhaps overflow) */
        return 2;
    }
    else
    {   /* string was all digits and converted properly */
        errno = 0;
        passwd = getpwuid(maybe_uid);
    }

    if (passwd != NULL)
    {
        *p_uid = passwd->pw_uid;
    }
    else
    {   /* check whether user wasn't found or an error occurred */
        if (errno == 0)
            retval = 1;
        else
            retval = 2;
    }

    return retval;
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
