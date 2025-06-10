# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
kde_enable_exceptions()
add_library(KPim6AkonadiSearchXapian)
add_library(KPim6::AkonadiSearchXapian ALIAS KPim6AkonadiSearchXapian)

target_sources(
    KPim6AkonadiSearchXapian
    PRIVATE
        xapiansearchstore.cpp
        xapiandocument.cpp
        xapiandatabase.cpp
        xapiantermgenerator.cpp
        xapianqueryparser.cpp
        xapiansearchstore.h
        xapiandocument.h
        xapiandatabase.h
        xapiantermgenerator.h
        xapianqueryparser.h
)

ecm_qt_declare_logging_category(KPim6AkonadiSearchXapian HEADER akonadi_search_xapian_debug.h IDENTIFIER AKONADI_SEARCH_XAPIAN_LOG CATEGORY_NAME org.kde.pim.akonadi_search_xapian
        DESCRIPTION "akonadisearch (akonadi indexer xapian)"
        OLD_CATEGORY_NAMES log_akonadi_search_xapian
        EXPORT AKONADISEARCH
)

if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim6AkonadiSearchXapian PROPERTIES UNITY_BUILD ON)
endif()

target_link_libraries(
    KPim6AkonadiSearchXapian
    PUBLIC Qt::Core KPim6::AkonadiSearchCore ${XAPIAN_LIBRARIES}
)

set_target_properties(
    KPim6AkonadiSearchXapian
    PROPERTIES
        VERSION ${AKONADISEARCH_VERSION}
        SOVERSION ${AKONADISEARCH_SOVERSION}
        EXPORT_NAME AkonadiSearchXapian
)

generate_export_header(
    KPim6AkonadiSearchXapian
    BASE_NAME AKONADI_SEARCH_XAPIAN
    EXPORT_FILE_NAME search_xapian_export.h
)

install(
    TARGETS KPim6AkonadiSearchXapian
    EXPORT KPim6AkonadiSearchTargets
    ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

install(
    FILES
        xapiandatabase.h
        xapiandocument.h
        xapianqueryparser.h
        xapiansearchstore.h
        xapiantermgenerator.h
        ${CMAKE_CURRENT_BINARY_DIR}/search_xapian_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiSearch/Xapian
    COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
