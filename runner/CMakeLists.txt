# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none

kcoreaddons_add_plugin(krunner_pimcontacts INSTALL_NAMESPACE "kf5/krunner")

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


kcoreaddons_desktop_to_json(krunner_pimcontacts plasma-krunner-pimcontacts.desktop)
target_link_libraries(krunner_pimcontacts
    KF5::Runner
    KF5::AkonadiCore
    KF5AkonadiSearchPIM
    KF5::I18n
    KF5::Contacts
    KF5::Codecs
)

kcoreaddons_add_plugin(kcm_krunner_pimcontacts INSTALL_NAMESPACE "kf5/krunner/kcms")

target_sources(kcm_krunner_pimcontacts PRIVATE
    pimcontactsrunnerconfig.cpp
    pimcontactsrunnerconfig.h
)

target_link_libraries(kcm_krunner_pimcontacts
    Qt${QT_MAJOR_VERSION}::Gui
    KF5::KCMUtils
    KF5::I18n
)
