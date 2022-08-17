add_library(contactsearchstore MODULE )
target_sources(contactsearchstore PRIVATE contactsearchstore.cpp ../pimsearchstore.cpp contactsearchstore.h ../pimsearchstore.h)
target_link_libraries(contactsearchstore
    Qt::Core
    KF5::AkonadiCore
    ${XAPIAN_LIBRARIES}
    KF5AkonadiSearchCore
    KF5AkonadiSearchXapian
)

install(TARGETS contactsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/akonadi)
