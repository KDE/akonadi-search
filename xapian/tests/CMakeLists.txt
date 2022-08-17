add_executable(writeiotest writeiotest.cpp)
target_link_libraries(writeiotest
    Qt::Core
    KF5::AkonadiSearchXapian
)

add_executable(memorytest memorytest.cpp)
target_link_libraries(memorytest
    Qt::Core
    KF5::AkonadiSearchXapian
)
