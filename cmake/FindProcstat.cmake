# - Try to find procstat library
# Once done this will define
#
#  PROCSTAT_FOUND - system has procstat
#  PROCSTAT_INCLUDE_DIR - the procstat include directory
#  PROCSTAT_LIBRARIES - The libraries needed to use procstat

# SPDX-FileCopyrightText: 2019 Tobias C. Berner <tcberner@FreeBSD.org>
#
# SPDX-License-Identifier: BSD-2-Clause


FIND_PATH(PROCSTAT_INCLUDE_DIR libprocstat.h)

FIND_LIBRARY(PROCSTAT_LIBRARIES NAMES procstat )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PROCSTAT DEFAULT_MSG PROCSTAT_INCLUDE_DIR PROCSTAT_LIBRARIES )

set_package_properties(PROCSTAT PROPERTIES
    URL "https://github.com/freebsd/freebsd/tree/master/lib/libprocstat"
    DESCRIPTION "Library to access process information"
)

MARK_AS_ADVANCED(PROCSTAT_INCLUDE_DIR PROCSTAT_LIBRARIES)
