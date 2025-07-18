# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(PIM_VERSION "6.5.40")
project(AkonadiSearch VERSION ${PIM_VERSION})

# ECM setup
set(KF_MIN_VERSION "6.15.0")

set(CMAKE_CXX_STANDARD 20)
find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include(CMakeDependentOption)
include(ECMSetupVersion)
file(
    GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES
    agent/*.cpp
    agent/*.h
    agent/*.c
    akonadiplugin/*.cpp
    akonadiplugin/*.h
    akonadiplugin/*.c
    core/*.cpp
    core/*.h
    core/*.c
    debug/*.cpp
    debug/*.h
    debug/*.c
    lib/*.cpp
    lib/*.h
    lib/*.c
    runner/*.cpp
    runner/*.h
    runner/*.c
    search/*.cpp
    search/*.h
    search/*.c
    xapian/*.cpp
    xapian/*.h
    xapian/*.c
)

set(PRE_COMMIT_INSTALLED FALSE)
if(${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
    if(EXISTS "${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit")
        file(READ "${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit" FILE_CONTENTS)
        string(FIND "${FILE_CONTENTS}" "File generated by pre-commit" INDEX)
        if(${INDEX} GREATER_EQUAL 0)
            set(PRE_COMMIT_INSTALLED TRUE)
        endif()
    endif()
endif()
if(NOT ${PRE_COMMIT_INSTALLED})
    include(KDEGitCommitHooks)
    include(KDEClangFormat)
    kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
endif()

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMAddTests)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMFeatureSummary)
include(ECMAddQch)

option(
    BUILD_QCH
    "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)"
    OFF
)
add_feature_info(
    QCH
    ${BUILD_QCH}
    "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)"
)

cmake_dependent_option(
    AKONADISEARCH_ENABLE_KRUNNER_RUNNER
    "Build KRunner runner to search contacts"
    ON
    "NOT WIN32;NOT APPLE"
    OFF
)

set(AKONADISEARCH_VERSION ${PIM_VERSION})

set(AKONADI_VERSION "6.5.40")
set(AKONADI_MIMELIB_VERSION "6.5.40")
set(KMIME_LIB_VERSION "6.5.40")
set(QT_REQUIRED_VERSION "6.8.0")
set(KPIM_MIME_VERSION "6.5.40")
set(KTEXTADDONS_MIN_VERSION "1.6.0")

find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Test)
find_package(
    KF6
    ${KF_MIN_VERSION}
    REQUIRED
    COMPONENTS I18n Config Crash DBusAddons KCMUtils KIO
)
if(AKONADISEARCH_ENABLE_KRUNNER_RUNNER)
    find_package(KF6Runner ${KF_MIN_VERSION} REQUIRED)
endif()

find_package(Xapian CONFIG)
set_package_properties(
    Xapian
    PROPERTIES
        DESCRIPTION "The Xapian search engine library"
        URL "https://xapian.org"
        TYPE REQUIRED
)

find_package(KPim6Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
find_package(KF6Contacts ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KPim6Mime ${KPIM_MIME_VERSION} CONFIG REQUIRED)
find_package(KPim6AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)
find_package(KF6CalendarCore ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6TextUtils ${KTEXTADDONS_MIN_VERSION} CONFIG REQUIRED)

find_package(Corrosion CONFIG)
set_package_properties(
    Corrosion
    PROPERTIES
        PURPOSE "Required for safe parsing of html emails"
        DESCRIPTION
            "CMake scripts to seamlessly build and link to targets using cargo"
        URL https://github.com/corrosion-rs/corrosion
)

ecm_setup_version(PROJECT
    VARIABLE_PREFIX AKONADISEARCH
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/akonadi_search_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim6AkonadiSearchConfigVersion.cmake"
    SOVERSION 6
)
option(KDEPIM_RUN_AKONADI_TEST "Enable autotest based on Akonadi." TRUE)

# Compiler flags
add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_search\")
add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
ecm_set_disabled_deprecation_versions(QT 6.10.0 KF 6.16.0)

option(
    USE_UNITY_CMAKE_SUPPORT
    "Use UNITY cmake support (speedup compile time)"
    OFF
)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if(USE_UNITY_CMAKE_SUPPORT)
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
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim6AkonadiSearch")
# Targets
add_subdirectory(xapian)
add_subdirectory(core)
add_subdirectory(agent)
add_subdirectory(lib)
add_subdirectory(akonadiplugin)
add_subdirectory(search)
add_subdirectory(debug)
if(AKONADISEARCH_ENABLE_KRUNNER_RUNNER)
    add_subdirectory(runner)
endif()

if(BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KPim6AkonadiSearchPIM_QCH
        FILE KPim6AkonadiSearchPIMQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS
        "include(\"\${CMAKE_CURRENT_LIST_DIR}/KPim6AkonadiSearchPIMQchTargets.cmake\")"
    )
endif()

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KPimAkonadiSearchConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KPim6AkonadiSearchConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KPim6AkonadiSearchConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KPim6AkonadiSearchConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT devel
)

install(
    EXPORT KPim6AkonadiSearchTargets
    NAMESPACE KPim6::
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KPim6AkonadiSearchTargets.cmake
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/akonadi_search_version.h"
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiSearch
    COMPONENT Devel
)

ecm_qt_install_logging_categories(
        EXPORT AKONADISEARCH
        FILE akonadi-search.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(DEFINED kde_configure_git_pre_commit_hook)
    kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
endif()
ki18n_install(po)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
