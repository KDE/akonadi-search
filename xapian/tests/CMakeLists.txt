add_executable(writeiotest writeiotest.cpp)
target_link_libraries(writeiotest Qt::Core KPim6::AkonadiSearchXapian)

add_executable(memorytest memorytest.cpp)
target_link_libraries(memorytest Qt::Core KPim6::AkonadiSearchXapian)
