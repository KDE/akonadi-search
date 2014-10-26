kde_enable_exceptions()

set(XAPIAN_SRCS
    xapiansearchstore.cpp
    xapiandocument.cpp
    xapiandatabase.cpp
    xapiantermgenerator.cpp
    xapianqueryparser.cpp
)

add_library(BalooPimXapian ${XAPIAN_SRCS})

target_link_libraries(BalooPimXapian PUBLIC
    Qt5::Core
    BalooPimCore
    ${XAPIAN_LIBRARIES}
)

set_target_properties(BalooPimXapian PROPERTIES
    VERSION ${AKONADISEARCH_VERSION_STRING}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME BalooPimXapian
)

generate_export_header(BalooPimXapian BASE_NAME BALOO_XAPIAN EXPORT_FILE_NAME xapian_export.h)

install(TARGETS BalooPimXapian EXPORT BalooPimXapian ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(autotests)
add_subdirectory(tests)