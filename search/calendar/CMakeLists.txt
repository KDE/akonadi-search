add_library(calendarsearchstore MODULE )
target_sources(calendarsearchstore PRIVATE calendarsearchstore.cpp ../pimsearchstore.cpp calendarsearchstore.h ../pimsearchstore.h)
target_link_libraries(calendarsearchstore
    Qt::Core
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    ${XAPIAN_LIBRARIES}
    KPim${KF_MAJOR_VERSION}AkonadiSearchCore
    KPim${KF_MAJOR_VERSION}AkonadiSearchXapian
)

install(TARGETS calendarsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/akonadi)
