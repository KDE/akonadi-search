add_library(contactsearchstore MODULE )
target_sources(contactsearchstore PRIVATE contactsearchstore.cpp ../pimsearchstore.cpp contactsearchstore.h ../pimsearchstore.h)
target_link_libraries(contactsearchstore
    Qt::Core
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    ${XAPIAN_LIBRARIES}
    KPim${KF_MAJOR_VERSION}AkonadiSearchCore
    KPim${KF_MAJOR_VERSION}AkonadiSearchXapian
)

install(TARGETS contactsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/akonadi)
