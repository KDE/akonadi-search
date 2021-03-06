kde_enable_exceptions()
include_directories(
    ${XAPIAN_INCLUDE_DIR}
)

add_library(KF5AkonadiSearchPIM)
add_library(KF5::AkonadiSearchPIM ALIAS KF5AkonadiSearchPIM)


target_sources(KF5AkonadiSearchPIM PRIVATE
    query.cpp
    resultiterator.cpp
    contactquery.cpp
    emailquery.cpp
    notequery.cpp
    contactcompleter.cpp
    collectionquery.cpp
    indexeditems.cpp

    ../search/email/agepostingsource.cpp
)

ecm_qt_declare_logging_category(KF5AkonadiSearchPIM HEADER akonadi_search_pim_debug.h IDENTIFIER AKONADI_SEARCH_PIM_LOG CATEGORY_NAME org.kde.pim.akonadi_search_pim
        DESCRIPTION "akonadisearch(akonadi pim lib)"
        OLD_CATEGORY_NAMES log_akonadi_search_pim
        EXPORT AKONADISEARCH
    )

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KF5AkonadiSearchPIM PROPERTIES UNITY_BUILD ON)
endif()


generate_export_header(KF5AkonadiSearchPIM BASE_NAME AKONADI_SEARCH_PIM EXPORT_FILE_NAME search_pim_export.h)

target_link_libraries(KF5AkonadiSearchPIM
    PUBLIC
    Qt::Core

    PRIVATE
    KF5::AkonadiCore
    ${XAPIAN_LIBRARIES}
)

set_target_properties(KF5AkonadiSearchPIM PROPERTIES
    VERSION ${AKONADISEARCH_VERSION}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearchPIM
)

target_include_directories(KF5AkonadiSearchPIM INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/AkonadiSearch>")

install(TARGETS KF5AkonadiSearchPIM EXPORT KF5AkonadiSearchTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    query.h
    contactquery.h
    emailquery.h
    contactcompleter.h
    notequery.h
    collectionquery.h
    indexeditems.h

    resultiterator.h
    ${CMAKE_CURRENT_BINARY_DIR}/search_pim_export.h

    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/AkonadiSearch/PIM
    COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
