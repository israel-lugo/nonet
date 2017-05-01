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


/* msg - message printing module */


#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <errno.h>

/* get basename() */
#include <libgen.h>

#include "common.h"


/* Program's basename, for printing on error. Initialized by msg_init(). */
const char *prog_name = NULL;


/*
 * Perform necessary initializations for the msg module.
 *
 * Receives the program's first command-line argument, argv[0]. This function
 * MAY modify the string. The string MUST NOT be otherwise modified or freed
 * while this module is expected to be used.
 *
 * This function MUST be called before calling any other function in this
 * module, and before accessing any publicly accessible variable from this
 * module (e.g. prog_name).
 */
void msg_init(char *argv0)
{
    prog_name = (const char *)basename(argv0);
}


/*
 * Terminate program with an error message.
 *
 * Receives a message, which will be printed to stderr along with the
 * program's name and a libc (errno) error description.
 *
 * msg_init() MUST have already been called at least once before.
 *
 * */
void die(const char *msg) __attribute__ ((__noreturn__));
void die(const char *msg)
{
    assert(prog_name != NULL);

    fprintf(stderr, "%s: %s: %s\n", prog_name, msg, strerror(errno));
    exit(2);
}


