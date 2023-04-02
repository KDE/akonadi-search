include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(emailindexer emailtest.cpp ../emailindexer.cpp ../abstractindexer.cpp ../akonadi_indexer_agent_debug.cpp ../stringutil.cpp)
target_link_libraries(emailindexer
  Qt::Test
  KPim${KF_MAJOR_VERSION}::AkonadiCore
  KPim${KF_MAJOR_VERSION}::AkonadiMime
  KPim${KF_MAJOR_VERSION}::Mime
  KPim${KF_MAJOR_VERSION}::AkonadiSearchPIM
  KPim${KF_MAJOR_VERSION}::AkonadiSearchXapian
  KF${KF_MAJOR_VERSION}::Codecs
  Qt::Widgets
)
