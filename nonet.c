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

#define _GNU_SOURCE
#include <sched.h>


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



#define COPYRIGHT "Copyright (C) 2016 Israel G. Lugo"
#define PACKAGE_NAME "nonet"
#define PACKAGE_VERSION "0.1"


extern char **environ;


static const char *prog_name = "<noname>";


struct cli_options {
    char * const * cmdline;
    uid_t uid;
    bool user_specified;
    bool allow_root;
};


static void die(const char *msg) __attribute__ ((__noreturn__));


/*
 * Terminate program upon error.
 *
 * Receives a message, which will be printed to stderr along with the
 * program's name and a libc (errno) error description.
 */
static void die(const char *msg)
{
    fprintf(stderr, "%s: %s: %s\n", prog_name, msg, strerror(errno));
    exit(2);
}


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
 * Change user in a safe way.
 *
 * Sets the effective, real and saved user id of the process to the
 * specified uid. Makes sure the uid's were all changed correctly and
 * aborts the program otherwise.
 */
static void set_user(uid_t uid)
{
    uid_t ruid, euid, suid;
    int status;

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
static void deal_with_uid(bool allow_root, bool user_specified, uid_t uid)
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
 * Execute the specified program, replacing the current process.
 *
 * argv is an array of argument strings, the first of which (argv[0]) is
 * the filename to execute. It must be terminated with a null pointer. If
 * clobber_env is true, the environment is emptied before running the
 * program. Exits on error.
 */
static void run_prog(char *const argv[], bool clobber_env)
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


/*
 * Get a UID from a username.
 *
 * Sets the uid_t pointed to by p_uid, if the user is found. Returns zero in
 * case of success, 1 if the user was not found, or 2 if there's an error in
 * errno.
 */
static int get_user(uid_t *p_uid, const char *username)
{
    struct passwd *passwd;
    int retval = 0;

    errno = 0;

    passwd = getpwnam(username);

    if (passwd != NULL)
    {
        *p_uid = passwd->pw_uid;
    }
    else
    {
        if (errno == 0)
            retval = 1;
        else
            retval = 2;
    }

    return retval;
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
    int status;


    prog_name = basename(argv[0]);

    parse_args(argc, argv, &cli_options);

    status = unshare(CLONE_NEWNET);
    if (status != 0)
        die("unable to disassociate from network namespace");

    deal_with_uid(cli_options.allow_root,
                  cli_options.user_specified,
                  cli_options.uid);

    run_prog(cli_options.cmdline, false);

    exit(0);
}

/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
