#! /bin/bash
#
# nonet - execute a command without network access
# Copyright 2016 Israel G. Lugo
#
# This file is part of nonet.
#

# Bootstrap the build system
#
# Should be used when first checking out the sources, to create
# the necessary generated files (configure, config.guess, etc)
#


# Try to use autoreconf if possible
if ( autoreconf --version > /dev/null 2>&1 ); then
  autoreconf -i
else
  # autoreconf not available, manually launch the tools
  aclocal && autoconf && automake -ac && autoheader
fi
