# TODO: add -Wall -Wextra and all this jazz
# TODO: build and run tests
# TODO: provide installation commands (did we have deinstallation, too?)
# TODO: check the old Makefile to see what I'm missing here
# TODO: cppcheck jobs = 5
# TODO: define GIT_HASH
cmake_minimum_required (VERSION 3.0)
project (newsboat)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wunreachable-code")
include(GNUInstallDirs)
# TODO: check that by default, this actually resolves into the same dir as before - /usr/local/share/locale
add_definitions(-DLOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

include_directories("${PROJECT_SOURCE_DIR}/filter")
add_subdirectory(filter)

include_directories("${PROJECT_SOURCE_DIR}/rss")
add_subdirectory(rss)

add_library(beuter
    ${PROJECT_SOURCE_DIR}/src/configcontainer.cpp
    ${PROJECT_SOURCE_DIR}/src/configparser.cpp
    ${PROJECT_SOURCE_DIR}/src/colormanager.cpp
    ${PROJECT_SOURCE_DIR}/src/keymap.cpp
    ${PROJECT_SOURCE_DIR}/src/stflpp.cpp
    ${PROJECT_SOURCE_DIR}/src/logger.cpp
    ${PROJECT_SOURCE_DIR}/src/exception.cpp
    ${PROJECT_SOURCE_DIR}/src/utils.cpp
    ${PROJECT_SOURCE_DIR}/src/matcher.cpp
    ${PROJECT_SOURCE_DIR}/src/formatstring.cpp
    ${PROJECT_SOURCE_DIR}/src/strprintf.cpp)

file(GLOB STFL_SRCS "${PROJECT_SOURCE_DIR}/stfl/*.stfl")
set(STFL_HEADERS)
foreach(SRC ${STFL_SRCS})
    string(REGEX REPLACE "stfl$" "h" OUTPUT_FILE_NAME ${SRC})

    add_custom_command(
        OUTPUT "${OUTPUT_FILE_NAME}"
        COMMAND ${PROJECT_SOURCE_DIR}/txt2h ${SRC} .stfl > ${OUTPUT_FILE_NAME}
        DEPENDS ${SRC}
        )
    set(STFL_HEADERS "${STFL_HEADERS}" "${OUTPUT_FILE_NAME}")
endforeach(SRC)
add_custom_target(
    dialog-headers
    DEPENDS ${STFL_HEADERS})

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/stfl"
    )

add_executable(newsboat newsbeuter.cpp
    "${PROJECT_SOURCE_DIR}/gitversion.cpp"
    "${PROJECT_SOURCE_DIR}/src/cache.cpp"
    "${PROJECT_SOURCE_DIR}/src/htmlrenderer.cpp"
    "${PROJECT_SOURCE_DIR}/src/urlreader.cpp"
    "${PROJECT_SOURCE_DIR}/src/logger.cpp"
    "${PROJECT_SOURCE_DIR}/src/view.cpp"
    "${PROJECT_SOURCE_DIR}/src/controller.cpp"
    "${PROJECT_SOURCE_DIR}/src/reloadthread.cpp"
    "${PROJECT_SOURCE_DIR}/src/tagsouppullparser.cpp"
    "${PROJECT_SOURCE_DIR}/src/downloadthread.cpp"
    "${PROJECT_SOURCE_DIR}/src/rss.cpp"
    "${PROJECT_SOURCE_DIR}/src/rss_parser.cpp"
    "${PROJECT_SOURCE_DIR}/src/formaction.cpp"
    "${PROJECT_SOURCE_DIR}/src/list_formaction.cpp"
    "${PROJECT_SOURCE_DIR}/src/feedlist_formaction.cpp"
    "${PROJECT_SOURCE_DIR}/src/itemlist_formaction.cpp"
    "${PROJECT_SOURCE_DIR}/src/itemview_formaction.cpp"
    "${PROJECT_SOURCE_DIR}/src/help_formaction.cpp"
    "${PROJECT_SOURCE_DIR}/src/filebrowser_formaction.cpp"
    "${PROJECT_SOURCE_DIR}/src/urlview_formaction.cpp"
    "${PROJECT_SOURCE_DIR}/src/select_formaction.cpp"
    "${PROJECT_SOURCE_DIR}/src/history.cpp"
    "${PROJECT_SOURCE_DIR}/src/filtercontainer.cpp"
    "${PROJECT_SOURCE_DIR}/src/listformatter.cpp"
    "${PROJECT_SOURCE_DIR}/src/regexmanager.cpp"
    "${PROJECT_SOURCE_DIR}/src/dialogs_formaction.cpp"
    "${PROJECT_SOURCE_DIR}/src/ttrss_api.cpp"
    "${PROJECT_SOURCE_DIR}/src/ttrss_urlreader.cpp"
    "${PROJECT_SOURCE_DIR}/src/newsblur_api.cpp"
    "${PROJECT_SOURCE_DIR}/src/newsblur_urlreader.cpp"
    "${PROJECT_SOURCE_DIR}/src/markreadthread.cpp"
    "${PROJECT_SOURCE_DIR}/src/oldreader_urlreader.cpp"
    "${PROJECT_SOURCE_DIR}/src/oldreader_api.cpp"
    "${PROJECT_SOURCE_DIR}/src/feedhq_api.cpp"
    "${PROJECT_SOURCE_DIR}/src/feedhq_urlreader.cpp"
    "${PROJECT_SOURCE_DIR}/src/textformatter.cpp"
    "${PROJECT_SOURCE_DIR}/src/ocnews_api.cpp"
    "${PROJECT_SOURCE_DIR}/src/ocnews_urlreader.cpp"
    "${PROJECT_SOURCE_DIR}/src/remote_api.cpp"
    )
target_link_libraries(newsboat filter rsspp beuter)
add_dependencies(newsboat dialog-headers)

add_custom_target(
    xlicense-header
    COMMAND ${PROJECT_SOURCE_DIR}/txt2h ${PROJECT_SOURCE_DIR}/LICENSE > ${PROJECT_SOURCE_DIR}/xlicense.h
    DEPENDS "${PROJECT_SOURCE_DIR}/LICENSE"
    )
add_dependencies(newsboat xlicense-header)

add_executable(podboat podbeuter.cpp
    "${PROJECT_SOURCE_DIR}/gitversion.cpp"
    "${PROJECT_SOURCE_DIR}/src/pb_controller.cpp"
    "${PROJECT_SOURCE_DIR}/src/pb_view.cpp"
    "${PROJECT_SOURCE_DIR}/src/download.cpp"
    "${PROJECT_SOURCE_DIR}/src/queueloader.cpp"
    "${PROJECT_SOURCE_DIR}/src/poddlthread.cpp"
    )
target_link_libraries(podboat filter rsspp beuter)
add_dependencies(podboat dialog-headers)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/Modules)

include(GetGitRevisionDescription)
git_describe(GIT_HASH --abbrev=4 --always --dirty)
if (GIT_HASH)
    set(GIT_HASH "-${GIT_HASH}")
endif (GIT_HASH)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/gitversion.cpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/gitversion.cpp"
    @ONLY)

find_package(LibXml2)
if (LIBXML2_FOUND)
    include_directories(${LIBXML2_INCLUDE_DIR})
    target_link_libraries (newsboat ${LIBXML2_LIBRARIES})
endif (LIBXML2_FOUND)

find_package(CURL)
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    target_link_libraries (newsboat ${CURL_LIBRARIES})
    target_link_libraries (podboat ${CURL_LIBRARIES})

    execute_process (COMMAND curl-config --static-libs
                     COMMAND egrep -- "-lssl( |$)"
                     RESULT_VARIABLE HAVE_OPENSSL
                     OUTPUT_QUIET
                     ERROR_QUIET)
    if (${HAVE_OPENSSL} STREQUAL "0")
        find_package(OpenSSL)
        if (OPENSSL_FOUND)
            include_directories(${OPENSSL_INCLUDE_DIRS})
            target_link_libraries (newsboat ${OPENSSL_LIBRARIES})
            target_link_libraries (podboat ${OPENSSL_LIBRARIES})
        endif (OPENSSL_FOUND)

        add_definitions(-DHAVE_OPENSSL=1)
    endif (${HAVE_OPENSSL} STREQUAL "0")

    execute_process (COMMAND curl-config --static-libs
                     COMMAND grep -- -lgcrypt
                     RESULT_VARIABLE HAVE_GCRYPT
                     OUTPUT_QUIET
                     ERROR_QUIET)
    if (${HAVE_GCRYPT} STREQUAL "0")
        find_package(GnuTLS)
        if (GNUTLS_FOUND)
            include_directories(${GNUTLS_INCLUDE_DIRS})
            target_link_libraries (newsboat ${GNUTLS_LIBRARIES})
            target_link_libraries (podboat ${GNUTLS_LIBRARIES})
        endif (GNUTLS_FOUND)

        add_definitions(-DHAVE_GCRYPT=1)
    endif (${HAVE_GCRYPT} STREQUAL "0")
endif (CURL_FOUND)

find_package(STFL)
if (STFL_FOUND)
    include_directories(${STFL_INCLUDE_DIRS})
    target_link_libraries (newsboat ${STFL_LIBRARIES})
    target_link_libraries (podboat ${STFL_LIBRARIES})
endif (STFL_FOUND)

find_package(SQLITE3)
if (SQLITE3_FOUND)
    include_directories(${SQLITE3_INCLUDE_DIRS})
    target_link_libraries (newsboat ${SQLITE3_LIBRARIES})
endif (SQLITE3_FOUND)

find_package(JSON-C)
if (JSON-C_FOUND)
    include_directories(${JSON-C_INCLUDE_DIRS})
    target_link_libraries (newsboat ${JSON-C_LIBRARIES})
endif (JSON-C_FOUND)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses)
if (CURSES_FOUND AND CURSES_HAVE_NCURSES_H)
    include_directories(${CURSES_INCLUDE_DIRS})
    target_link_libraries (newsboat ${CURSES_LIBRARIES})
    target_link_libraries (podboat ${CURSES_LIBRARIES})
endif (CURSES_FOUND AND CURSES_HAVE_NCURSES_H)
