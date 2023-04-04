add_library(notesearchstore MODULE)
target_sources(notesearchstore PRIVATE notesearchstore.cpp ../pimsearchstore.cpp notesearchstore.h ../pimsearchstore.h)
target_link_libraries(notesearchstore
    Qt::Core
    KPim6::AkonadiCore
    ${XAPIAN_LIBRARIES}
    KPim6AkonadiSearchCore
    KPim6AkonadiSearchXapian
)

install(TARGETS notesearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/akonadi)
