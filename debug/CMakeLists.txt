# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
add_library(KPim${KF_MAJOR_VERSION}AkonadiSearchDebug)
add_library(KPim${KF_MAJOR_VERSION}::AkonadiSearchDebug ALIAS KPim${KF_MAJOR_VERSION}AkonadiSearchDebug)

target_sources(KPim${KF_MAJOR_VERSION}AkonadiSearchDebug PRIVATE
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

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim${KF_MAJOR_VERSION}AkonadiSearchDebug PROPERTIES UNITY_BUILD ON)
endif()


generate_export_header(KPim${KF_MAJOR_VERSION}AkonadiSearchDebug BASE_NAME AKONADI_SEARCH_DEBUG EXPORT_FILE_NAME search_debug_export.h)

target_link_libraries(KPim${KF_MAJOR_VERSION}AkonadiSearchDebug
    PRIVATE
    Qt::Widgets
    KF${KF_MAJOR_VERSION}::I18n
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KF${KF_MAJOR_VERSION}::WidgetsAddons
    KF${KF_MAJOR_VERSION}::ConfigCore
    KF${KF_MAJOR_VERSION}::Completion
    KF${KF_MAJOR_VERSION}::ConfigGui
)
if (QT_MAJOR_VERSION STREQUAL "6")
target_link_libraries(KPim${KF_MAJOR_VERSION}AkonadiSearchDebug
    PRIVATE
    Qt6::Core5Compat
    )
endif()
set_target_properties(KPim${KF_MAJOR_VERSION}AkonadiSearchDebug PROPERTIES
    VERSION ${AKONADISEARCH_VERSION}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearchDebug
)

target_include_directories(KPim${KF_MAJOR_VERSION}AkonadiSearchDebug INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/AkonadiSearch>")

install(TARGETS KPim${KF_MAJOR_VERSION}AkonadiSearchDebug EXPORT KPim${KF_MAJOR_VERSION}AkonadiSearchTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    akonadisearchdebugdialog.h
    akonadisearchdebugsearchpathcombobox.h
    ${CMAKE_CURRENT_BINARY_DIR}/search_debug_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/AkonadiSearch/Debug
    COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
