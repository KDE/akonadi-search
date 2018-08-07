set(contactcompletiontest_SRCS
    contactcompletiontest.cpp
)
add_executable(contactcompletiontest ${contactcompletiontest_SRCS})
target_link_libraries(contactcompletiontest
    Qt5::Core
    KF5::AkonadiSearch
)
