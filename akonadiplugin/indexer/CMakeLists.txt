kde_enable_exceptions()

set(akonadi_indexing_plugin_SRCS indexingplugin.cpp)
ecm_qt_declare_logging_category(akonadi_indexing_plugin_SRCS
    HEADER akonadiplugin_indexing_debug.h
    IDENTIFIER AKONADIPLUGIN_INDEXING_LOG
    CATEGORY_NAME org.kde.pim.akonadiplugin_indexing)

add_library(akonadi_indexing_plugin
 MODULE
    ${akonadi_indexing_plugin_SRCS}
)

target_link_libraries(akonadi_indexing_plugin
    KF5::AkonadiCore
    KF5::AkonadiSearch
)

install(TARGETS akonadi_indexing_plugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/akonadi)

if(BUILD_TESTING)
#    add_subdirectory(autotests)
endif()

