# - Try to find procstat library
# Once done this will define
#
#  PROCSTAT_FOUND - system has procstat
#  PROCSTAT_INCLUDE_DIR - the procstat include directory
#  PROCSTAT_LIBRARIES - The libraries needed to use procstat

# Copyright (c) 2019, Tobias C. Berner <tcberner@FreeBSD.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.


FIND_PATH(PROCSTAT_INCLUDE_DIR libprocstat.h)

FIND_LIBRARY(PROCSTAT_LIBRARIES NAMES procstat )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PROCSTAT DEFAULT_MSG PROCSTAT_INCLUDE_DIR PROCSTAT_LIBRARIES )

set_package_properties(PROCSTAT PROPERTIES
    URL "https://github.com/freebsd/freebsd/tree/master/lib/libprocstat"
    DESCRIPTION "Library to access process information"
)

MARK_AS_ADVANCED(PROCSTAT_INCLUDE_DIR PROCSTAT_LIBRARIES)
