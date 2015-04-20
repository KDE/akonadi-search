add_library(calendarsearchstore MODULE calendarsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(calendarsearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    KF5AkonadiSearchCore
    KF5AkonadiSearchXapian
)

install(TARGETS calendarsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/akonadi)
