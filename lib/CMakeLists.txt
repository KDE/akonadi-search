# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
kde_enable_exceptions()
include_directories(${XAPIAN_INCLUDE_DIR})

add_library(KPim6AkonadiSearchPIM)
add_library(KPim6::AkonadiSearchPIM ALIAS KPim6AkonadiSearchPIM)

target_sources(
    KPim6AkonadiSearchPIM
    PRIVATE
        query.cpp
        resultiterator.cpp
        contactquery.cpp
        emailquery.cpp
        notequery.cpp
        contactcompleter.cpp
        collectionquery.cpp
        indexeditems.cpp
        ../search/email/agepostingsource.cpp
        query.h
        resultiterator.h
        contactquery.h
        emailquery.h
        notequery.h
        contactcompleter.h
        collectionquery.h
        indexeditems.h
        ../search/email/agepostingsource.h
)

ecm_qt_declare_logging_category(KPim6AkonadiSearchPIM HEADER akonadi_search_pim_debug.h IDENTIFIER AKONADI_SEARCH_PIM_LOG CATEGORY_NAME org.kde.pim.akonadi_search_pim
        DESCRIPTION "akonadisearch(akonadi pim lib)"
        OLD_CATEGORY_NAMES log_akonadi_search_pim
        EXPORT AKONADISEARCH
)

if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim6AkonadiSearchPIM PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(
    KPim6AkonadiSearchPIM
    BASE_NAME AKONADI_SEARCH_PIM
    EXPORT_FILE_NAME search_pim_export.h
)

target_link_libraries(
    KPim6AkonadiSearchPIM
    PUBLIC Qt::Core
    PRIVATE KPim6::AkonadiCore ${XAPIAN_LIBRARIES}
)

set_target_properties(
    KPim6AkonadiSearchPIM
    PROPERTIES
        VERSION ${AKONADISEARCH_VERSION}
        SOVERSION ${AKONADISEARCH_SOVERSION}
        EXPORT_NAME AkonadiSearchPIM
)

target_include_directories(
    KPim6AkonadiSearchPIM
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiSearch>"
)

install(
    TARGETS KPim6AkonadiSearchPIM
    EXPORT KPim6AkonadiSearchTargets
    ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

set(AKONADISEARCH_INCLUDES
    query.h
    contactquery.h
    emailquery.h
    contactcompleter.h
    notequery.h
    collectionquery.h
    indexeditems.h
    resultiterator.h
)

install(
    FILES
        ${AKONADISEARCH_INCLUDES}
        ${CMAKE_CURRENT_BINARY_DIR}/search_pim_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiSearch/PIM
    COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
