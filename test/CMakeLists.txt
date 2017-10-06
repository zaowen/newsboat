include_directories("${PROJECT_SOURCE_DIR}/test")

add_executable(catch_tests
    ${NEWSBOAT_SOURCES}
    "${PROJECT_SOURCE_DIR}/test/test.cpp"
    "${PROJECT_SOURCE_DIR}/test/cache.cpp"
    "${PROJECT_SOURCE_DIR}/test/configcontainer.cpp"
    "${PROJECT_SOURCE_DIR}/test/configparser.cpp"
    "${PROJECT_SOURCE_DIR}/test/filterparser.cpp"
    "${PROJECT_SOURCE_DIR}/test/formatstring.cpp"
    "${PROJECT_SOURCE_DIR}/test/history.cpp"
    "${PROJECT_SOURCE_DIR}/test/htmlrenderer.cpp"
    "${PROJECT_SOURCE_DIR}/test/itemlist_formaction.cpp"
    "${PROJECT_SOURCE_DIR}/test/keymap.cpp"
    "${PROJECT_SOURCE_DIR}/test/listformatter.cpp"
    "${PROJECT_SOURCE_DIR}/test/matcher.cpp"
    "${PROJECT_SOURCE_DIR}/test/regexmanager.cpp"
    "${PROJECT_SOURCE_DIR}/test/remote_api.cpp"
    "${PROJECT_SOURCE_DIR}/test/rss.cpp"
    "${PROJECT_SOURCE_DIR}/test/strprintf.cpp"
    "${PROJECT_SOURCE_DIR}/test/tagsouppullparser.cpp"
    "${PROJECT_SOURCE_DIR}/test/textformatter.cpp"
    "${PROJECT_SOURCE_DIR}/test/urlreader.cpp"
    "${PROJECT_SOURCE_DIR}/test/utils.cpp"
    )
add_dependencies(catch_tests dialog-headers)
target_link_libraries(catch_tests filter rsspp beuter)

if (STFL_FOUND)
    target_link_libraries (catch_tests ${STFL_LIBRARIES})
endif (STFL_FOUND)
if (CURL_FOUND)
    target_link_libraries (catch_tests ${CURL_LIBRARIES})

    if (${HAVE_OPENSSL} STREQUAL "0")
        if (OPENSSL_FOUND)
            target_link_libraries (catch_tests ${OPENSSL_LIBRARIES})
        endif (OPENSSL_FOUND)
    endif (${HAVE_OPENSSL} STREQUAL "0")

    if (${HAVE_GCRYPT} STREQUAL "0")
        if (GNUTLS_FOUND)
            target_link_libraries (catch_tests ${GNUTLS_LIBRARIES})
        endif (GNUTLS_FOUND)
    endif (${HAVE_GCRYPT} STREQUAL "0")
endif (CURL_FOUND)

if (JSON-C_FOUND)
    target_link_libraries (catch_tests ${JSON-C_LIBRARIES})
endif (JSON-C_FOUND)

if (SQLITE3_FOUND)
    target_link_libraries (catch_tests ${SQLITE3_LIBRARIES})
endif (SQLITE3_FOUND)

if (CURSES_FOUND AND CURSES_HAVE_NCURSES_H)
    target_link_libraries (catch_tests ${CURSES_LIBRARIES})
endif (CURSES_FOUND AND CURSES_HAVE_NCURSES_H)
