add_library(emailsearchstore MODULE )
target_sources(emailsearchstore PRIVATE agepostingsource.cpp emailsearchstore.cpp ../pimsearchstore.cpp agepostingsource.h emailsearchstore.h ../pimsearchstore.h)
target_link_libraries(emailsearchstore
    Qt::Core
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    ${XAPIAN_LIBRARIES}
    KPim${KF_MAJOR_VERSION}AkonadiSearchCore
    KPim${KF_MAJOR_VERSION}AkonadiSearchXapian
)

install(TARGETS emailsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/akonadi)
