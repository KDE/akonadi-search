ecm_add_test(termgeneratortest.cpp
    TEST_NAME "termgeneratortest"
    LINK_LIBRARIES Qt::Test KF5::AkonadiSearchXapian
)

ecm_add_test(queryparsertest.cpp
    TEST_NAME "queryparsertest"
    LINK_LIBRARIES Qt::Test KF5::AkonadiSearchXapian
)
