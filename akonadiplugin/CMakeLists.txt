kde_enable_exceptions()

add_library(akonadi_search_plugin
 MODULE
    searchplugin.cpp
    akonadiplugin_indexer_debug.cpp
)

target_link_libraries(akonadi_search_plugin
    KF5::AkonadiCore
    KF5::AkonadiMime
    ${XAPIAN_LIBRARIES}
    KF5::AkonadiSearchCore
    KF5::AkonadiSearchPIM
    KF5::AkonadiSearchXapian
    KF5::ItemModels
    KF5::Contacts
)

install(TARGETS akonadi_search_plugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/akonadi)

add_subdirectory(autotests)
