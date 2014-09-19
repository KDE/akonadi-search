ecm_add_test(termgeneratortest.cpp
    TEST_NAME "termgeneratortest"
    LINK_LIBRARIES Qt5::Test BalooPimXapian
)

ecm_add_test(queryparsertest.cpp
    TEST_NAME "queryparsertest"
    LINK_LIBRARIES Qt5::Test BalooPimXapian
)
