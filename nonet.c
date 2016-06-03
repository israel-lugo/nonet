#define _GNU_SOURCE
#include <sched.h>


#include <unistd.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <getopt.h>



extern char **environ;


char *prog_name = "<noname>";

static void die(char *const msg) __attribute__ ((__noreturn__));

static void die(char *const msg)
{
    fprintf(stderr, "%s: %s: %s\n", prog_name, msg, strerror(errno));
    exit(2);
}


static void set_user(uid_t uid) {
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


static void maybe_drop_priv(void) {
    uid_t ruid = getuid();
    uid_t euid = geteuid();

    if (ruid != euid)
    {
        set_user(ruid);
    }
}


static void run_prog(char *const argv[], bool clobber_env) {
    static const char *const empty = { NULL };

    if (clobber_env)
    {   /* this kills our own env, but we're about to exec anyway */
        environ = (char **)&empty;
    }

    execvp(argv[0], argv);
    fprintf(stderr, "%s: unable to run program: %s\n", prog_name, strerror(errno));
    exit(1);
}


int main(int argc, char *argv[]) {

    int status;

    prog_name = basename(argv[0]);

    status = unshare(CLONE_NEWNET);
    if (status != 0)
        die("unable to disassociate from network namespace");

    maybe_drop_priv();

    if (argc > 1)
    {
        run_prog(argv+1, false);
    }

    exit(0);
}
