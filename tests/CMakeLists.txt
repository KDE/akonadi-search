set(contactcompletiontest_SRCS
    contactcompletiontest.cpp
)
add_executable(contactcompletiontest ${contactcompletiontest_SRCS})
target_link_libraries(contactcompletiontest
    Qt::Core
    KPim6::AkonadiSearch
)
