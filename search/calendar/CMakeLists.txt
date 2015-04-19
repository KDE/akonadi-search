add_library(calendarsearchstore MODULE calendarsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(calendarsearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    BalooPimCore
    BalooPimXapian
)

install(TARGETS calendarsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/baloo)
