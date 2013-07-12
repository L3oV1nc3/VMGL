# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

# This file lets one set various compile-time options.

# Set RELEASE to 1 to compile with optimizations and without debug info.
RELEASE=1

# Set THREADSAFE to 1 if you want thread safety for parallel applications.
THREADSAFE=0

# Set to 1 if you want to force building 32-bit objects on a 64-bit system.
FORCE_32BIT_ABI=0

# Set USE_OSMESA to 1 if you want to enable off screen rendering using Mesa.
USE_OSMESA=0
