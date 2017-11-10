kde_enable_exceptions()

set(akonadi_search_plugin_SRCS searchplugin.cpp)
ecm_qt_declare_logging_category(akonadi_search_plugin_SRCS
    HEADER akonadiplugin_search_debug.h
    IDENTIFIER AKONADIPLUGIN_SEARCH_LOG
    CATEGORY_NAME org.kde.pim.akonadiplugin_search
)

add_library(akonadi_search_plugin
 MODULE
    ${akonadi_search_plugin_SRCS}
)

target_link_libraries(akonadi_search_plugin
    KF5::AkonadiCore
    KF5::AkonadiSearch
)

install(TARGETS akonadi_search_plugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/akonadi)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
