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


/* cli.c - command-line interface */


#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <getopt.h>

#include "common.h"
#include "msg.h"
#include "nonet.h"
#include "uid.h"


#define COPYRIGHT "Copyright (C) 2016 Israel G. Lugo"



struct cli_options {
    char * const * cmdline;
    uid_t uid;
    bool user_specified;
    bool allow_root;
};


/*
 * Show command-line options with pretty formatting.
 *
 * To be used from within show_usage.
 */
static void show_options(void)
{
    static const struct { const char *name; const char *desc; } opts[] = {
        { "-a, --allow-root", "allow running command as root user" },
        { "-u, --user=USERNAME", "run command as specified user name" },
        { "-h, --help", "display this help and exit" },
        { "-v, --version", "output version information and exit" },
        { "--", "stop processing command line options" }
    };

    unsigned int i;
    for (i=0; i < sizeof(opts)/sizeof(opts[0]); i++)
    {
        printf("  %-28s%s\n", opts[i].name, opts[i].desc);
    }
}


/*
 * Show usage information.
 */
static void show_usage(void)
{
    printf("%s (%s) %s - execute a command without network access\n"
           "%s\n"
           "\n"
           "This program executes a command in its own separate network namespace,\n"
           "thus giving it a separate isolated view of the networking stack. It will\n"
           "see no network interfaces, IP routing tables, firewall rules, and so on.\n"
           "\n",
           prog_name,
           PACKAGE_NAME,
           PACKAGE_VERSION,
           COPYRIGHT);
    printf(" Usage:\n"
           "  %s [OPTIONS] [--] <command> [<args>...]\n"
           "\n"
           "\n"
           "OPTIONS:\n",
           prog_name);
    show_options();
}


/*
 * Show version information.
 */
static void show_version_info(void)
{
    printf("%s %s\n"
           "%s\n"
           "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
           "This is free software: you are free to change and redistribute it.\n"
           "There is NO WARRANTY, to the extent permitted by law.\n",
           PACKAGE_NAME,
           PACKAGE_VERSION,
           COPYRIGHT);
}


/*
 * Print a string directing the user to the help functionality.
 *
 * To be used whenever the user inputs an invalid argument.
 */
static void print_help_string(void)
{
    fprintf(stderr, "Try '%s --help' for more information.\n", prog_name);
}


/*
 * Process command-line arguments.
 *
 * Receives the number of arguments (argc), the array of arguments (argv)
 * and a pointer to a struct cli_options which will be modified as per the
 * specified command-line arguments. The struct cli_options should already
 * be initialized to safe defaults. Exits on error.
 */
static void parse_args(int argc, char *const argv[],
        struct cli_options *p_cli_options)
{
    static const struct option long_opts[] = {
        {"allow-root", 0, 0, 'a'},
        {"help", 0, 0, 'h'},
        {"user", 1, 0, 'u'},
        {"version", 0, 0, 'v'},
        {0, 0, 0, 0}
    };
    int status;

    for (;;)
    {
        int arg = getopt_long(argc, argv, "ahu:v", long_opts, NULL);

        if (arg == -1)
            break;

        switch (arg)
        {
            case 'a':   /* --allow-root */
                p_cli_options->allow_root = true;
                break;
            case 'h':   /* --help */
                show_usage();
                exit(0);
            case 'u':   /* --user <username> */
                status = get_user(&p_cli_options->uid, optarg);
                if (status == 1)
                {
                    fprintf(stderr, "%s: unknown user %s\n", prog_name, optarg);
                    exit(2);
                }
                if (status == 2)
                    die("unable to lookup user");

                p_cli_options->user_specified = true;
                break;
            case 'v':   /* --version */
                show_version_info();
                exit(0);
            default:    /* invalid option or missing mandatory argument */
                print_help_string();
                exit(2);
        }
    }

    if (optind >= argc)
    {
        fprintf(stderr, "%s: missing command\n", prog_name);
        print_help_string();
        exit(2);
    }

    p_cli_options->cmdline = &argv[optind];
}


int main(int argc, char *argv[])
{
    struct cli_options cli_options = { 
        .uid = 0, .allow_root = false, .user_specified = false
    };

    msg_init(argv[0]);

    parse_args(argc, argv, &cli_options);

    lose_net();

    deal_with_uid(cli_options.allow_root,
                  cli_options.user_specified,
                  cli_options.uid);

    run_prog(cli_options.cmdline, false);

    exit(0);
}

/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
