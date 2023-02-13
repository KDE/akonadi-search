# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(PIM_VERSION "5.22.40")
project(AkonadiSearch VERSION ${PIM_VERSION})

# ECM setup
set(KF_MIN_VERSION "5.103.0")

find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include(CMakeDependentOption)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMAddTests)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMAddQch)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")


cmake_dependent_option(
    AKONADISEARCH_ENABLE_KRUNNER_RUNNER
    "Build KRunner runner to search contacts" ON
    "NOT WIN32" OFF
)

set(AKONADISEARCH_VERSION ${PIM_VERSION})

set(AKONADI_VERSION "5.22.40")
set(AKONADI_MIMELIB_VERSION "5.22.40")
set(KMIME_LIB_VERSION "5.22.40")
set(QT_REQUIRED_VERSION "5.15.2")
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_MIN_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Test)
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS I18n Config Crash DBusAddons KCMUtils KIO)
if (AKONADISEARCH_ENABLE_KRUNNER_RUNNER)
    find_package(KF${KF_MAJOR_VERSION}Runner ${KF_MIN_VERSION} REQUIRED)
endif()
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat)
endif()


find_package(Xapian CONFIG)
set_package_properties(Xapian PROPERTIES
    DESCRIPTION "The Xapian search engine library"
    URL "https://xapian.org"
    TYPE REQUIRED
)

find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}Contacts ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}CalendarCore ${KF_MIN_VERSION} CONFIG REQUIRED)

ecm_setup_version(PROJECT
    VARIABLE_PREFIX AKONADISEARCH
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/akonadi_search_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiSearchConfigVersion.cmake"
    SOVERSION 5
)
option(KDEPIM_RUN_AKONADI_TEST "Enable autotest based on Akonadi." TRUE)

# Compiler flags
add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_search\")
ecm_set_disabled_deprecation_versions(QT 6.4 KF 5.102.0)



option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
    add_definitions(-DCOMPILE_WITH_UNITY_CMAKE_SUPPORT)
endif()


# Turn exceptions on
kde_enable_exceptions()

include_directories(
  ${XAPIAN_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/core
  ${CMAKE_BINARY_DIR}/core
  ${CMAKE_SOURCE_DIR}/xapian
  ${CMAKE_BINARY_DIR}/xapian
)
# Config files
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5AkonadiSearch")

# Targets
add_subdirectory(xapian)
add_subdirectory(core)
add_subdirectory(agent)
add_subdirectory(lib)
add_subdirectory(akonadiplugin)
add_subdirectory(search)
add_subdirectory(debug)
if (AKONADISEARCH_ENABLE_KRUNNER_RUNNER)
    add_subdirectory(runner)
endif()


configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF5AkonadiSearchConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiSearchConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiSearchConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiSearchConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT devel
)

install(EXPORT KF5AkonadiSearchTargets
        NAMESPACE KF5::
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5AkonadiSearchTargets.cmake)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/akonadi_search_version.h"
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/AkonadiSearch
    COMPONENT Devel
)

ecm_qt_install_logging_categories(
        EXPORT AKONADISEARCH
        FILE akonadi-search.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
feature_summary(WHAT ALL
    INCLUDE_QUIET_PACKAGES
    FATAL_ON_MISSING_REQUIRED_PACKAGES
)

