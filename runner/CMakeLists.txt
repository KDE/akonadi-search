
ecm_qt_declare_logging_category(pimcontactsrunner_DEBUG HEADER akonadi_runner_debug.h
        IDENTIFIER AKONADI_KRUNNER_LOG
        CATEGORY_NAME org.kde.pim.akonadi_krunner_plugin
        DESCRIPTION "akonadisearch (akonadi plugin krunner)"
        OLD_CATEGORY_NAMES log_akonadi_krunner_plugin
        EXPORT AKONADISEARCH
    )


set(pimcontactsrunner_SRCS
    pimcontactsrunner.cpp
    ${pimcontactsrunner_DEBUG}
)

add_library(krunner_pimcontacts MODULE ${pimcontactsrunner_SRCS})
kcoreaddons_desktop_to_json(krunner_pimcontacts plasma-krunner-pimcontacts.desktop)
target_link_libraries(krunner_pimcontacts
    KF5::Runner
    KF5::AkonadiCore
    KF5AkonadiSearchPIM
    KF5::I18n
    KF5::Contacts
    KF5::Codecs
)

########################
add_library(kcm_krunner_pimcontacts MODULE)
target_sources(kcm_krunner_pimcontacts PRIVATE
    pimcontactsrunnerconfig.cpp
    ${pimcontactsrunner_DEBUG}
)

target_link_libraries(kcm_krunner_pimcontacts
    Qt::Gui
    KF5::KCMUtils
    KF5::I18n
)

install(TARGETS krunner_pimcontacts DESTINATION "${KDE_INSTALL_PLUGINDIR}/kf5/krunner")
install(TARGETS 
                kcm_krunner_pimcontacts
        DESTINATION ${KDE_INSTALL_PLUGINDIR}
)
install(FILES 
              plasma-krunner-pimcontacts_config.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
)

