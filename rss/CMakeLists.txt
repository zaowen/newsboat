add_library(rsspp
    atom_parser.cpp
    exception.cpp
    parser.cpp
    parser_factory.cpp
    rss_09x_parser.cpp
    rss_10_parser.cpp
    rss_parser.cpp)

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/stfl"
    "${PROJECT_SOURCE_DIR}/rss"
    )

find_package(LibXml2)
if (LIBXML2_FOUND)
    include_directories(${LIBXML2_INCLUDE_DIR})
    target_link_libraries (rsspp ${LIBXML2_LIBRARIES})
endif (LIBXML2_FOUND)
