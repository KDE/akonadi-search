cmake_minimum_required(VERSION 3.0)

set(PIM_VERSION "5.9.40")
project(AkonadiSearch VERSION ${PIM_VERSION})

# ECM setup
set(KF5_VERSION "5.50.0")

find_package(ECM ${KF5_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMAddTests)
include(ECMQtDeclareLoggingCategory)
include(ECMCoverageOption)

set(AKONADISEARCH_VERSION ${PIM_VERSION})

set(AKONADI_VERSION "5.9.40")
set(AKONADI_MIMELIB_VERSION "5.9.40")
set(KCONTACTS_LIB_VERSION "5.9.40")
set(KCALENDARCORE_LIB_VERSION "5.9.40")
set(KMIME_LIB_VERSION "5.9.40")
set(QT_REQUIRED_VERSION "5.9.0")

find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Test)
find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS I18n Config Crash KCMUtils)
if (NOT WIN32)
    find_package(KF5Runner ${KF5_VERSION} REQUIRED)
endif()


find_package(Xapian CONFIG)
set_package_properties(Xapian PROPERTIES
    DESCRIPTION "The Xapian search engine library"
    URL "http://xapian.org"
    TYPE REQUIRED
)

find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarCore ${KCALENDARCORE_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)

ecm_setup_version(PROJECT
    VARIABLE_PREFIX AKONADISEARCH
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/akonadi_search_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiSearchConfigVersion.cmake"
    SOVERSION 5
)

# Compiler flags
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_search\")
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
#add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)
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

# Targets
add_subdirectory(xapian)
add_subdirectory(core)
add_subdirectory(agent)
add_subdirectory(lib)
add_subdirectory(akonadiplugin)
add_subdirectory(search)
add_subdirectory(debug)
if (NOT WIN32)
    add_subdirectory(runner)
endif()

# Config files
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5AkonadiSearch")

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
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}
    COMPONENT Devel
)

install( FILES akonadi-search.renamecategories akonadi-search.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

feature_summary(WHAT ALL
    INCLUDE_QUIET_PACKAGES
    FATAL_ON_MISSING_REQUIRED_PACKAGES
)

