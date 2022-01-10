add_library(notesearchstore MODULE)
target_sources(notesearchstore PRIVATE notesearchstore.cpp ../pimsearchstore.cpp notesearchstore.h ../pimsearchstore.h)
target_link_libraries(notesearchstore
    Qt${QT_MAJOR_VERSION}::Core
    KF5::AkonadiCore
    ${XAPIAN_LIBRARIES}
    KF5AkonadiSearchCore
    KF5AkonadiSearchXapian
)

install(TARGETS notesearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/akonadi)
