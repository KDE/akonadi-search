
if (ECM_VERSION VERSION_LESS "5.68.0")
    ecm_qt_declare_logging_category(pimcontactsrunner_DEBUG HEADER akonadi_runner_debug.h
        IDENTIFIER AKONADI_KRUNNER_LOG
        CATEGORY_NAME org.kde.pim.akonadi_krunner_plugin
    )
else()
    ecm_qt_declare_logging_category(pimcontactsrunner_DEBUG HEADER akonadi_runner_debug.h
        IDENTIFIER AKONADI_KRUNNER_LOG
        CATEGORY_NAME org.kde.pim.akonadi_krunner_plugin
        DESCRIPTION "akonadisearch (akonadi plugin krunner)"
        OLD_CATEGORY_NAMES log_akonadi_krunner_plugin
        EXPORT AKONADISEARCH
    )
endif()


set(pimcontactsrunner_SRCS
    pimcontactsrunner.cpp
    ${pimcontactsrunner_DEBUG}
)

add_library(krunner_pimcontacts MODULE ${pimcontactsrunner_SRCS})
target_link_libraries(krunner_pimcontacts
    KF5::Runner
    KF5::AkonadiCore
    KF5AkonadiSearchPIM
    KF5::I18n
    KF5::Contacts
    KF5::Codecs
)

set(pimcontactsrunnerconfig_SRCS
    pimcontactsrunnerconfig.cpp
    ${pimcontactsrunner_DEBUG}
)

add_library(kcm_krunner_pimcontacts MODULE ${pimcontactsrunnerconfig_SRCS})
target_link_libraries(kcm_krunner_pimcontacts
    Qt5::Gui
    KF5::KCMUtils
    KF5::I18n
)


install(TARGETS krunner_pimcontacts
                kcm_krunner_pimcontacts
        DESTINATION ${PLUGIN_INSTALL_DIR}
)
install(FILES plasma-krunner-pimcontacts.desktop
              plasma-krunner-pimcontacts_config.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
)

