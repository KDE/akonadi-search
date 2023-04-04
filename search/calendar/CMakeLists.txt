add_library(calendarsearchstore MODULE )
target_sources(calendarsearchstore PRIVATE calendarsearchstore.cpp ../pimsearchstore.cpp calendarsearchstore.h ../pimsearchstore.h)
target_link_libraries(calendarsearchstore
    Qt::Core
    KPim6::AkonadiCore
    ${XAPIAN_LIBRARIES}
    KPim6AkonadiSearchCore
    KPim6AkonadiSearchXapian
)

install(TARGETS calendarsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/akonadi)
