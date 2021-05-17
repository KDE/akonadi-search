kde_enable_exceptions()

set(CORE_SRCS
    searchstore.cpp
    term.cpp
    query.cpp
    resultiterator.cpp
)

ecm_qt_declare_logging_category(CORE_SRCS HEADER akonadi_search_core_debug.h IDENTIFIER AKONADI_SEARCH_CORE_LOG CATEGORY_NAME org.kde.pim.akonadi_search_core
        DESCRIPTION "akonadisearch (akonadi core lib)"
        EXPORT AKONADISEARCH
    )



add_library(KF5AkonadiSearchCore ${CORE_SRCS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KF5AkonadiSearchCore PROPERTIES UNITY_BUILD ON)
endif()

add_library(KF5::AkonadiSearchCore ALIAS KF5AkonadiSearchCore)

target_link_libraries(KF5AkonadiSearchCore
    PUBLIC
    Qt::Core
)

set_target_properties(KF5AkonadiSearchCore PROPERTIES
    VERSION ${AKONADISEARCH_VERSION}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearchCore
)
target_include_directories(KF5AkonadiSearchCore INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/AkonadiSearch/Core>")

generate_export_header(KF5AkonadiSearchCore BASE_NAME AKONADI_SEARCH_CORE EXPORT_FILE_NAME search_core_export.h)

install(TARGETS KF5AkonadiSearchCore EXPORT KF5AkonadiSearchTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    searchstore.h
    term.h
    query.h
    resultiterator.h
    ${CMAKE_CURRENT_BINARY_DIR}/search_core_export.h

    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/AkonadiSearch/Core
    COMPONENT Devel
)
