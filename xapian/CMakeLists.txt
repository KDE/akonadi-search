kde_enable_exceptions()

set(XAPIAN_SRCS
    xapiansearchstore.cpp
    xapiandocument.cpp
    xapiandatabase.cpp
    xapiantermgenerator.cpp
    xapianqueryparser.cpp
)

ecm_qt_declare_logging_category(XAPIAN_SRCS HEADER akonadi_search_xapian_debug.h IDENTIFIER AKONADI_SEARCH_XAPIAN_LOG CATEGORY_NAME org.kde.pim.akonadi_search_xapian)

add_library(KF5AkonadiSearchXapian ${XAPIAN_SRCS})
add_library(KF5::AkonadiSearchXapian ALIAS KF5AkonadiSearchXapian)

target_link_libraries(KF5AkonadiSearchXapian PUBLIC
    Qt5::Core
    KF5::AkonadiSearchCore
    ${XAPIAN_LIBRARIES}
)

set_target_properties(KF5AkonadiSearchXapian PROPERTIES
    VERSION ${AKONADISEARCH_VERSION_STRING}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearchXapian
)

generate_export_header(KF5AkonadiSearchXapian BASE_NAME AKONADI_SEARCH_XAPIAN EXPORT_FILE_NAME search_xapian_export.h)

install(TARGETS KF5AkonadiSearchXapian EXPORT AkonadiSearchXapian ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
