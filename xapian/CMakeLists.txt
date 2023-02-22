# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
kde_enable_exceptions()
add_library(KPim${KF_MAJOR_VERSION}AkonadiSearchXapian)
add_library(KPim${KF_MAJOR_VERSION}::AkonadiSearchXapian ALIAS KPim${KF_MAJOR_VERSION}AkonadiSearchXapian)

target_sources(KPim${KF_MAJOR_VERSION}AkonadiSearchXapian PRIVATE
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

ecm_qt_declare_logging_category(KPim${KF_MAJOR_VERSION}AkonadiSearchXapian HEADER akonadi_search_xapian_debug.h IDENTIFIER AKONADI_SEARCH_XAPIAN_LOG CATEGORY_NAME org.kde.pim.akonadi_search_xapian
        DESCRIPTION "akonadisearch (akonadi indexer xapian)"
        OLD_CATEGORY_NAMES log_akonadi_search_xapian
        EXPORT AKONADISEARCH
    )


if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim${KF_MAJOR_VERSION}AkonadiSearchXapian PROPERTIES UNITY_BUILD ON)
endif()


target_link_libraries(KPim${KF_MAJOR_VERSION}AkonadiSearchXapian PUBLIC
    Qt::Core
    KPim${KF_MAJOR_VERSION}::AkonadiSearchCore
    ${XAPIAN_LIBRARIES}
)

set_target_properties(KPim${KF_MAJOR_VERSION}AkonadiSearchXapian PROPERTIES
    VERSION ${AKONADISEARCH_VERSION}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearchXapian
)

generate_export_header(KPim${KF_MAJOR_VERSION}AkonadiSearchXapian BASE_NAME AKONADI_SEARCH_XAPIAN EXPORT_FILE_NAME search_xapian_export.h)

install(TARGETS KPim${KF_MAJOR_VERSION}AkonadiSearchXapian EXPORT KPim${KF_MAJOR_VERSION}AkonadiSearchTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    xapiandatabase.h
    xapiandocument.h
    xapianqueryparser.h
    xapiansearchstore.h
    xapiantermgenerator.h
    ${CMAKE_CURRENT_BINARY_DIR}/search_xapian_export.h

    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/AkonadiSearch/Xapian
    COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
