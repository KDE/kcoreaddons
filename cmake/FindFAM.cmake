# - Try to find the FAM directory notification library
# Once done this will define
#
#  FAM_FOUND - system has FAM
#  FAM_INCLUDE_DIR - the FAM include directory
#  FAM_LIBRARIES - The libraries needed to use FAM

# SPDX-FileCopyrightText: 2006 Alexander Neundorf <neundorf@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause


FIND_PATH(FAM_INCLUDE_DIR fam.h)

FIND_LIBRARY(FAM_LIBRARIES NAMES fam )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FAM DEFAULT_MSG FAM_INCLUDE_DIR FAM_LIBRARIES )

set_package_properties(FAM PROPERTIES
    URL "http://oss.sgi.com/projects/fam"
    DESCRIPTION "File alteration notification support via a separate service"
)

MARK_AS_ADVANCED(FAM_INCLUDE_DIR FAM_LIBRARIES)

