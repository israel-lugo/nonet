# nonet

[![license](https://img.shields.io/badge/license-GPLv3+-blue.svg)](LICENSE)

Execute a command without network access.

This program executes a command in a completely isolated network namespace,
making sure it cannot access the network in any way.

## How it works

The command will have a separate isolated view of the entire network stack. It
will see no network interface devices, no IP addresses, no IP routing tables,
no firewall rules, and so on.

This is done at a low level, using Linux namespaces. In essence, the command is
run inside its own "network container".

The use of Linux namespaces does mean that **nonet** needs to be run as the
superuser (root). More specifically, it needs to have the `CAP_SYS_ADMIN`
capability, which root has by default. Also, **nonet** only runs on Linux.

## Questions and answers

### The whole point is I don't trust the command, I don't want to run it as root!

No problem. In fact, although **nonet** itself must be run as root, it will by
default refuse to run the command as root, unless you give it a specific option
to allow it (`--allow-root`).

The idea is to specify (using `--user`) the user with which **nonet** will run
the command.

### Do I always have to specify the user? Can't it be automatic?

The user can be automatically detected if the **nonet** binary is installed
_setuid root_ (meaning it can be run directly as root, without using `sudo` or
`su`).

If **nonet** is installed _setuid root_, it will by default run the command as
the same original user who ran **nonet**. It is still possible to manually
specify a user, with the aforementioned `--user` option.
