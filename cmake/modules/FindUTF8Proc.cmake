INCLUDE(FindPackageHandleStandardArgs)

IF (NOT UTF8PROC_INCLUDE_DIR)
    FIND_PATH(UTF8PROC_INCLUDE_DIR utf8proc.h HINTS $ENV{UTF8PROC_DIR})
ENDIF()

IF (NOT UTF8PROC_LIBRARIES)
    FIND_LIBRARY(UTF8PROC_LIBRARIES utf8proc HINTS $ENV{UTF8PROC_DIR})
ENDIF()

MARK_AS_ADVANCED(UTF8PROC_INCLUDE_DIR)
MARK_AS_ADVANCED(UTF8PROC_LIBRARIES)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    UTF8PROC
    DEFAULT_MSG
    UTF8PROC_LIBRARIES
    UTF8PROC_INCLUDE_DIR
)
