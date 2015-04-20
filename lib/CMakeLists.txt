kde_enable_exceptions()
include_directories(
    ${XAPIAN_INCLUDE_DIR}
)

set(AKONADI_SEARCH_PIM_LIB_SRCS
    query.cpp
    resultiterator.cpp
    contactquery.cpp
    emailquery.cpp
    notequery.cpp
    contactcompleter.cpp
    collectionquery.cpp

    ../search/email/agepostingsource.cpp
)

add_library(KF5AkonadiSearchPIM ${AKONADI_SEARCH_PIM_LIB_SRCS})
add_library(KF5::AkonadiSearchPIM ALIAS KF5AkonadiSearchPIM)

generate_export_header(KF5AkonadiSearchPIM BASE_NAME AKONADI_SEARCH_PIM EXPORT_FILE_NAME search_pim_export.h)

target_link_libraries(KF5AkonadiSearchPIM
    PUBLIC
    Qt5::Core

    PRIVATE
    KF5::AkonadiCore
    KF5::ItemModels
    ${XAPIAN_LIBRARIES}
)

set_target_properties(KF5AkonadiSearchPIM PROPERTIES
    VERSION ${AKONADISEARCH_VERSION_STRING}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearchPIM
)

target_include_directories(KF5AkonadiSearchPIM INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/AkonadiSearch>")

install(TARGETS KF5AkonadiSearchPIM EXPORT AkonadiSearchTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    query.h
    contactquery.h
    emailquery.h
    contactcompleter.h
    notequery.h
    collectionquery.h

    resultiterator.h
    ${CMAKE_CURRENT_BINARY_DIR}/search_pim_export.h

    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/AkonadiSearch/PIM
    COMPONENT Devel
)

add_subdirectory(tests)
