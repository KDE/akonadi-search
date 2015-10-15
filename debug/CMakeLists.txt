
set(akonadisearchdebug_SRCS
  akonadisearchdebugdialog.cpp
  akonadisearchdebugwidget.cpp
  job/akonadisearchdebugsearchjob.cpp
  akonadisearchdebugsearchpathcombobox.cpp
  akonadisearchsyntaxhighlighter.cpp

)

add_library(KF5AkonadiSearchDebug ${akonadisearchdebug_SRCS})
add_library(KF5::AkonadiSearchDebug ALIAS KF5AkonadiSearchDebug)

generate_export_header(KF5AkonadiSearchDebug BASE_NAME AKONADI_SEARCH_DEBUG EXPORT_FILE_NAME search_debug_export.h)

target_link_libraries(KF5AkonadiSearchDebug
    PRIVATE
    Qt5::Widgets
    KF5::I18n
    KF5::AkonadiCore
    KF5::WidgetsAddons
    KF5::ConfigCore
    KF5::Completion
)

set_target_properties(KF5AkonadiSearchDebug PROPERTIES
    VERSION ${AKONADISEARCH_VERSION_STRING}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearchDebug
)

target_include_directories(KF5AkonadiSearchDebug INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/AkonadiSearch>")

install(TARGETS KF5AkonadiSearchDebug EXPORT KF5AkonadiSearchTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    akonadisearchdebugdialog.h
    akonadisearchdebugsearchpathcombobox.h
    ${CMAKE_CURRENT_BINARY_DIR}/search_debug_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/AkonadiSearch/Debug
    COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
