kde_enable_exceptions()

set(CORE_SRCS
    searchstore.cpp
    term.cpp
    query.cpp
    resultiterator.cpp
)

add_library(BalooPimCore ${CORE_SRCS})

target_link_libraries(BalooPimCore
    PUBLIC
    Qt5::Core
)

set_target_properties(BalooPimCore PROPERTIES
    VERSION ${AKONADISEARCH_VERSION_STRING}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME BalooPimCore
)
target_include_directories(BalooPimCore INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/Baloo>")

generate_export_header(BalooPimCore BASE_NAME BALOO_CORE EXPORT_FILE_NAME core_export.h)

install(TARGETS BalooPimCore EXPORT AkonadiSearchTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

#install(FILES
#    ${CMAKE_CURRENT_BINARY_DIR}/core_export.h
#    ${KF5BalooCore_HEADERS}
#    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Baloo/baloo
#    COMPONENT Devel
#)
#
#install(FILES
#    ${KF5BalooCore_CamelCase_HEADERS}
#    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Baloo/Baloo
#    COMPONENT Devel
#)
