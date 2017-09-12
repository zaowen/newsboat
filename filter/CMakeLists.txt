add_library(filter
    FilterParser.cpp
    Parser.cpp
    Scanner.cpp)

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/stfl"
    "${PROJECT_SOURCE_DIR}/rss"
    )

find_package(LibXml2)
if (LIBXML2_FOUND)
    include_directories(${LIBXML2_INCLUDE_DIR})
    target_link_libraries (filter ${LIBXML2_LIBRARIES})
endif (LIBXML2_FOUND)
