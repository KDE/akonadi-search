add_library(calendarsearchstore MODULE )
target_sources(calendarsearchstore PRIVATE calendarsearchstore.cpp ../pimsearchstore.cpp)
target_link_libraries(calendarsearchstore
    Qt::Core
    KF5::AkonadiCore
    ${XAPIAN_LIBRARIES}
    KF5AkonadiSearchCore
    KF5AkonadiSearchXapian
)

install(TARGETS calendarsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/akonadi)
