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



extern char **environ;


char *prog_name = "<noname>";


struct cli_options {
    char * const * cmdline;
    uid_t uid;
    bool user_specified;
    bool allow_root;
};


static void die(char *const msg) __attribute__ ((__noreturn__));




static void die(char *const msg)
{
    fprintf(stderr, "%s: %s: %s\n", prog_name, msg, strerror(errno));
    exit(2);
}


static void show_help(void)
{
    /* TODO */
}


static void show_version_info(void)
{
    /* TODO */
}


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


static void run_prog(char *const argv[], bool clobber_env)
{
    static const char *const empty = { NULL };

    if (clobber_env)
    {   /* this kills our own env, but we're about to exec anyway */
        environ = (char **)&empty;
    }

    execvp(argv[0], argv);
    fprintf(stderr, "%s: unable to run program: %s\n", prog_name, strerror(errno));
    exit(1);
}


/*
 * Get a UID from a username.
 *
 * Sets the uid_t pointed to by p_uid, if the user is found. Returns zero in
 * case of success, 1 if the user was not found, or 2 if there's an error in
 * errno.
 */
static int get_user(uid_t *p_uid, char *const username)
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
                show_help();
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
        }
    }

    if (optind >= argc)
    {
        fprintf(stderr, "%s: missing command\n", prog_name);
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
