kde_enable_exceptions()

add_library(akonadi_baloo_searchplugin
 MODULE 
    searchplugin.cpp
)

target_link_libraries(akonadi_baloo_searchplugin
    KF5::AkonadiCore
    KF5::AkonadiMime
    ${XAPIAN_LIBRARIES}
    BalooPimCore
    BalooPimXapian
    KF5::BalooPim
    KF5::KDELibs4Support
    KF5::ItemModels
    KF5::Contacts
    )

install(TARGETS akonadi_baloo_searchplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/akonadi)
install(FILES akonadibaloosearchplugin.desktop
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/akonadi)


#REACTIVATE
add_subdirectory(autotests)
