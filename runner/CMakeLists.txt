# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
configure_file(plasma-krunner-pimcontacts.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/plasma-krunner-pimcontacts.json @ONLY)
kcoreaddons_add_plugin(krunner_pimcontacts INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/krunner")

ecm_qt_declare_logging_category(krunner_pimcontacts HEADER akonadi_runner_debug.h
    IDENTIFIER AKONADI_KRUNNER_LOG
    CATEGORY_NAME org.kde.pim.akonadi_krunner_plugin
    DESCRIPTION "akonadisearch (akonadi plugin krunner)"
    OLD_CATEGORY_NAMES log_akonadi_krunner_plugin
    EXPORT AKONADISEARCH
)
target_sources(krunner_pimcontacts PRIVATE
    pimcontactsrunner.cpp
    pimcontactsrunner.h
)

target_link_libraries(krunner_pimcontacts
    KF${KF_MAJOR_VERSION}::Runner
    KF5::AkonadiCore
    KF5AkonadiSearchPIM
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::ConfigCore
    KF${KF_MAJOR_VERSION}::Contacts
    KF${KF_MAJOR_VERSION}::Codecs
)

kcoreaddons_add_plugin(kcm_krunner_pimcontacts INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/krunner/kcms")

target_sources(kcm_krunner_pimcontacts PRIVATE
    pimcontactsrunnerconfig.cpp
    pimcontactsrunnerconfig.h
)

target_link_libraries(kcm_krunner_pimcontacts
    Qt::Gui
    KF${KF_MAJOR_VERSION}::KCMUtils
    KF${KF_MAJOR_VERSION}::I18n
)
