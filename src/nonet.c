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

/* get unshare() */
#define _GNU_SOURCE
#include <sched.h>


#include <unistd.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "msg.h"



extern char **environ;



/*
 * Disassociate from global network namespace.
 *
 * Requires root privileges to run. Dies in case of error.
 */
void lose_net(void)
{
    int status;

    status = unshare(CLONE_NEWNET);

    if (status != 0)
        die("unable to disassociate from network namespace");
}


/*
 * Execute the specified program, replacing the current process.
 *
 * argv is an array of argument strings, the first of which (argv[0]) is
 * the filename to execute. It must be terminated with a null pointer. If
 * clobber_env is true, the environment is emptied before running the
 * program. Exits on error.
 */
void run_prog(char *const argv[], bool clobber_env)
{
    static const char *const empty = { NULL };

    if (clobber_env)
    {   /* this kills our own env, but we're about to exec anyway */
        environ = (char **)&empty;
    }

    execvp(argv[0], argv);
    fprintf(stderr, "%s: unable to run '%s': %s\n", prog_name, argv[0],
            strerror(errno));
    exit(1);
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
