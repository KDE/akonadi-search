kde_enable_exceptions()

add_library(akonadi_search_plugin MODULE)
target_sources(
    akonadi_search_plugin
    PRIVATE
        searchplugin.cpp
        searchplugin.h
)
ecm_qt_declare_logging_category(akonadi_search_plugin HEADER akonadiplugin_search_debug.h IDENTIFIER AKONADIPLUGIN_SEARCH_LOG CATEGORY_NAME org.kde.pim.akonadiplugin_search
        DESCRIPTION "akonadisearch (akonadi search plugin)"
        OLD_CATEGORY_NAMES log_akonadiplugin_indexer org.kde.pim.akonadiplugin_indexer
        EXPORT AKONADISEARCH
)

target_link_libraries(
    akonadi_search_plugin
    KPim6::AkonadiCore
    KF5::AkonadiSearch
)

install(TARGETS akonadi_search_plugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/akonadi)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
