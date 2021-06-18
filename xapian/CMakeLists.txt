kde_enable_exceptions()
add_library(KF5AkonadiSearchXapian)
add_library(KF5::AkonadiSearchXapian ALIAS KF5AkonadiSearchXapian)

target_sources(KF5AkonadiSearchXapian PRIVATE
    xapiansearchstore.cpp
    xapiandocument.cpp
    xapiandatabase.cpp
    xapiantermgenerator.cpp
    xapianqueryparser.cpp
)

ecm_qt_declare_logging_category(KF5AkonadiSearchXapian HEADER akonadi_search_xapian_debug.h IDENTIFIER AKONADI_SEARCH_XAPIAN_LOG CATEGORY_NAME org.kde.pim.akonadi_search_xapian
        DESCRIPTION "akonadisearch (akonadi indexer xapian)"
        OLD_CATEGORY_NAMES log_akonadi_search_xapian
        EXPORT AKONADISEARCH
    )


if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KF5AkonadiSearchXapian PROPERTIES UNITY_BUILD ON)
endif()


target_link_libraries(KF5AkonadiSearchXapian PUBLIC
    Qt::Core
    KF5::AkonadiSearchCore
    ${XAPIAN_LIBRARIES}
)

set_target_properties(KF5AkonadiSearchXapian PROPERTIES
    VERSION ${AKONADISEARCH_VERSION}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearchXapian
)

generate_export_header(KF5AkonadiSearchXapian BASE_NAME AKONADI_SEARCH_XAPIAN EXPORT_FILE_NAME search_xapian_export.h)

install(TARGETS KF5AkonadiSearchXapian EXPORT KF5AkonadiSearchTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    xapiandatabase.h
    xapiandocument.h
    xapianqueryparser.h
    xapiansearchstore.h
    xapiantermgenerator.h
    ${CMAKE_CURRENT_BINARY_DIR}/search_xapian_export.h

    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/AkonadiSearch/Xapian
    COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
