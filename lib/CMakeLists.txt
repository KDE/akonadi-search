kde_enable_exceptions()
include_directories(
    ${XAPIAN_INCLUDE_DIR}
)

set(BALOO_PIM_LIB_SRCS
    query.cpp
    resultiterator.cpp
    contactquery.cpp
    emailquery.cpp
    notequery.cpp
    contactcompleter.cpp
    collectionquery.cpp

    ../search/email/agepostingsource.cpp
)

add_library(KF5BalooPim ${BALOO_PIM_LIB_SRCS})
add_library(KF5::BalooPim ALIAS KF5BalooPim)

generate_export_header(KF5BalooPim BASE_NAME BALOO_PIM EXPORT_FILE_NAME pim_export.h)

target_link_libraries(KF5BalooPim
    PUBLIC
    Qt5::Core

    PRIVATE
    KF5::KDELibs4Support
    KF5::AkonadiCore
    KF5::ItemModels
    ${XAPIAN_LIBRARIES}
)

set_target_properties(KF5BalooPim PROPERTIES
    VERSION ${AKONADISEARCH_VERSION_STRING}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME BalooPim
)

target_include_directories(KF5BalooPim INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/Baloo>")

install(TARGETS KF5BalooPim EXPORT AkonadiSearchTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    query.h
    contactquery.h
    emailquery.h
    contactcompleter.h
    notequery.h
    collectionquery.h

    resultiterator.h
    ${CMAKE_CURRENT_BINARY_DIR}/pim_export.h

    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/Baloo/baloo/pim
    COMPONENT Devel
)

add_subdirectory(tests)
