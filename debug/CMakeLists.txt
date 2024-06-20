# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
add_library(KPim6AkonadiSearchDebug)
add_library(KPim6::AkonadiSearchDebug ALIAS KPim6AkonadiSearchDebug)

target_sources(KPim6AkonadiSearchDebug PRIVATE
  akonadisearchdebugdialog.cpp
  akonadisearchdebugwidget.cpp
  job/akonadisearchdebugsearchjob.cpp
  akonadisearchdebugsearchpathcombobox.cpp
  akonadisearchsyntaxhighlighter.cpp

  akonadisearchdebugdialog.h
  akonadisearchdebugwidget.h
  job/akonadisearchdebugsearchjob.h
  akonadisearchdebugsearchpathcombobox.h
  akonadisearchsyntaxhighlighter.h
)

if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim6AkonadiSearchDebug PROPERTIES UNITY_BUILD ON)
endif()


generate_export_header(KPim6AkonadiSearchDebug BASE_NAME AKONADI_SEARCH_DEBUG EXPORT_FILE_NAME search_debug_export.h)

target_link_libraries(KPim6AkonadiSearchDebug
    PRIVATE
    Qt::Widgets
    KF6::I18n
    KPim6::AkonadiCore
    KF6::WidgetsAddons
    KF6::ConfigCore
    KF6::Completion
    KF6::ConfigGui
)
set_target_properties(KPim6AkonadiSearchDebug PROPERTIES
    VERSION ${AKONADISEARCH_VERSION}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearchDebug
)

target_include_directories(KPim6AkonadiSearchDebug INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiSearch>")

install(TARGETS KPim6AkonadiSearchDebug EXPORT KPim6AkonadiSearchTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    akonadisearchdebugdialog.h
    akonadisearchdebugsearchpathcombobox.h
    ${CMAKE_CURRENT_BINARY_DIR}/search_debug_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiSearch/Debug
    COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
