# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
kde_enable_exceptions()
add_library(KPim${KF_MAJOR_VERSION}AkonadiSearchCore)
add_library(KPim${KF_MAJOR_VERSION}::AkonadiSearchCore ALIAS KPim${KF_MAJOR_VERSION}AkonadiSearchCore)
target_sources(KPim${KF_MAJOR_VERSION}AkonadiSearchCore PRIVATE
    searchstore.cpp
    term.cpp
    query.cpp
    resultiterator.cpp

    searchstore.h
    term.h
    query.h
    resultiterator.h
    )

ecm_qt_declare_logging_category(KPim${KF_MAJOR_VERSION}AkonadiSearchCore HEADER akonadi_search_core_debug.h IDENTIFIER AKONADI_SEARCH_CORE_LOG CATEGORY_NAME org.kde.pim.akonadi_search_core
        DESCRIPTION "akonadisearch (akonadi core lib)"
        EXPORT AKONADISEARCH
    )

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim${KF_MAJOR_VERSION}AkonadiSearchCore PROPERTIES UNITY_BUILD ON)
endif()



target_link_libraries(KPim${KF_MAJOR_VERSION}AkonadiSearchCore
    PUBLIC
    Qt::Core
)

set_target_properties(KPim${KF_MAJOR_VERSION}AkonadiSearchCore PROPERTIES
    VERSION ${AKONADISEARCH_VERSION}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearchCore
)
target_include_directories(KPim${KF_MAJOR_VERSION}AkonadiSearchCore INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/AkonadiSearch>")

generate_export_header(KPim${KF_MAJOR_VERSION}AkonadiSearchCore BASE_NAME AKONADI_SEARCH_CORE EXPORT_FILE_NAME search_core_export.h)

ecm_generate_headers(KPim${KF_MAJOR_VERSION}AkonadiSearchCore_CamelCasecore_HEADERS
    HEADER_NAMES
    Term
    Query
    ResultIterator
    SearchStore
    REQUIRED_HEADERS KPim${KF_MAJOR_VERSION}AkonadiSearchCore_core_HEADERS
    PREFIX Core
    )

install(TARGETS KPim${KF_MAJOR_VERSION}AkonadiSearchCore EXPORT KPim${KF_MAJOR_VERSION}AkonadiSearchTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${KPim${KF_MAJOR_VERSION}AkonadiSearchCore_CamelCasecore_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/AkonadiSearch/Core
    COMPONENT Devel
)
install(FILES
    ${KPim${KF_MAJOR_VERSION}AkonadiSearchCore_core_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/AkonadiSearch/core
    COMPONENT Devel
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/search_core_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/AkonadiSearch
    COMPONENT Devel
)
