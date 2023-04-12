# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
kcoreaddons_add_plugin(krunner_pimcontacts INSTALL_NAMESPACE "kf6/krunner")

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
    KF6::Runner
    KPim6::AkonadiCore
    KPim6::AkonadiSearchPIM
    KF6::I18n
    KF6::ConfigCore
    KF6::Contacts
    KF6::Codecs
)

kcoreaddons_add_plugin(kcm_krunner_pimcontacts INSTALL_NAMESPACE "kf6/krunner/kcms")

target_sources(kcm_krunner_pimcontacts PRIVATE
    pimcontactsrunnerconfig.cpp
    pimcontactsrunnerconfig.h
)

target_link_libraries(kcm_krunner_pimcontacts
    Qt::Gui
    KF6::KCMUtils
    KF6::I18n
)
