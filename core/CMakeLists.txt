# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
kde_enable_exceptions()
add_library(KPim6AkonadiSearchCore)
add_library(KPim6::AkonadiSearchCore ALIAS KPim6AkonadiSearchCore)
target_sources(
    KPim6AkonadiSearchCore
    PRIVATE
        searchstore.cpp
        term.cpp
        query.cpp
        resultiterator.cpp
        searchstore.h
        term.h
        query.h
        resultiterator.h
)

ecm_qt_declare_logging_category(KPim6AkonadiSearchCore HEADER akonadi_search_core_debug.h IDENTIFIER AKONADI_SEARCH_CORE_LOG CATEGORY_NAME org.kde.pim.akonadi_search_core
        DESCRIPTION "akonadisearch (akonadi core lib)"
        EXPORT AKONADISEARCH
)

if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(
        KPim6AkonadiSearchCore
        PROPERTIES
            UNITY_BUILD
                ON
    )
endif()

target_link_libraries(KPim6AkonadiSearchCore PUBLIC Qt::Core)

set_target_properties(
    KPim6AkonadiSearchCore
    PROPERTIES
        VERSION
            ${AKONADISEARCH_VERSION}
        SOVERSION
            ${AKONADISEARCH_SOVERSION}
        EXPORT_NAME
            AkonadiSearchCore
)
target_include_directories(
    KPim6AkonadiSearchCore
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiSearch>"
)

generate_export_header(KPim6AkonadiSearchCore BASE_NAME AKONADI_SEARCH_CORE EXPORT_FILE_NAME search_core_export.h)

ecm_generate_headers(KPim6AkonadiSearchCore_CamelCasecore_HEADERS
    HEADER_NAMES
    Term
    Query
    ResultIterator
    SearchStore
    REQUIRED_HEADERS KPim6AkonadiSearchCore_core_HEADERS
    PREFIX Core
)

install(TARGETS KPim6AkonadiSearchCore EXPORT KPim6AkonadiSearchTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES
        ${KPim6AkonadiSearchCore_CamelCasecore_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiSearch/Core
    COMPONENT Devel
)
install(
    FILES
        ${KPim6AkonadiSearchCore_core_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiSearch/core
    COMPONENT Devel
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/search_core_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiSearch
    COMPONENT Devel
)
