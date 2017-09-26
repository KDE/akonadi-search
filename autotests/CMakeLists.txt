include(ECMAddTests)

function(add_test)
    set(options)
    set(oneValueArgs TEST_NAME)
    set(multiValueArgs LINK_LIBRARIES)
    cmake_parse_arguments(add_test "${options}" ${oneValueArgs}" ${multiValueArgs}" ${ARGN})
    message(STATUS "XX ${add_test_TEST_NAME}")
    ecm_add_test(${add_test_UNPARSED_ARGUMENTS}
        LINK_LIBRARIES Qt5::Core Qt5::Test ${add_test_LINK_LIBRARIES}
        TEST_NAME ${add_test_TEST_NAME}
        PREFIX -libs)
endfunction()

add_test(querypropertymappertest.cpp ../src/querypropertymapper.cpp
    TEST_NAME querypropertytestmapper
)

