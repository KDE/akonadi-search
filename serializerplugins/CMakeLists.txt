set(akonadi_serializer_collection_SRCS akonadi_serializer_collection.cpp)
add_library(akonadi_serializer_collection MODULE ${SERIALIZER_TYPE} ${akonadi_serializer_collection_SRCS})
target_link_libraries(akonadi_serializer_collection
    KF5::AkonadiCore
    KF5::AkonadiSearch
)
install(TARGETS akonadi_serializer_collection DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES
    akonadi_serializer_collection.desktop
DESTINATION ${KDE_INSTALL_DATADIR}/akonadi/plugins/serializer)
