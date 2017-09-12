# STFL_FOUND - true if library and headers were found
# STFL_INCLUDE_DIRS - include directories
# STFL_LIBRARIES - library directories

find_package(PkgConfig)
pkg_check_modules(PC_STFL QUIET stfl)

find_path(STFL_INCLUDE_DIR stfl.h
    HINTS ${PC_STFL_INCLUDEDIR} ${PC_STFL_INCLUDE_DIRS} PATH_SUFFIXES stfl)

find_library(STFL_LIBRARY NAMES stfl libstfl
    HINTS ${PC_STFL_LIBDIR} ${PC_STFL_LIBRARY_DIRS})

set(STFL_LIBRARIES ${STFL_LIBRARY})
set(STFL_INCLUDE_DIRS ${STFL_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(STFL DEFAULT_MSG STFL_LIBRARY STFL_INCLUDE_DIR)

mark_as_advanced(STFL_INCLUDE_DIR STFL_LIBRARY)
