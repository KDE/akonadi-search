include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(emailindexer emailtest.cpp ../emailindexer.cpp ../abstractindexer.cpp ../akonadi_indexer_agent_debug.cpp ../stringutil.cpp
${CMAKE_CURRENT_BINARY_DIR}/../../agent/akonadi_indexer_agent_email_debug.cpp)
target_link_libraries(emailindexer
  Qt::Test
  KPim6::AkonadiCore
  KPim6::AkonadiMime
  KPim6::Mime
  KPim6::AkonadiSearchPIM
  KPim6::AkonadiSearchXapian
  KF6::Codecs
  Qt::Widgets
)
