#include <gtest/gtest.h>
#include <morkparser.h>
#include "TestResources.h"

using namespace testing;

TEST(MailMorkParser, correctUnreadCount) {
    std::pair<const char*, unsigned int> cases[] = {
            std::make_pair("6_Unread_Inbox.msf", 6),
            std::make_pair("1_Unread_Filter.msf", 1),
            std::make_pair("0_Unread_Trash.msf", 0),
            std::make_pair("1_Unread_Inbox_Large.msf", 1),
            std::make_pair("1_Unread_Inbox_Duplicate_cells.msf", 1),
    };
    for (const auto testCase : cases) {
        MailMorkParser parser;
        QString path = TestResources::getAbsoluteResourcePath(std::get<0>(testCase));
        unsigned int expectedUnreadCount = std::get<1>(testCase);
        if (!parser.open(path)) {
            ADD_FAILURE() << "Expected the MailMorkParser to be able to open " << qPrintable(path);
            continue;
        }
        EXPECT_EQ(parser.getNumUnreadMessages(), expectedUnreadCount)
                        << "Expected the MailMorkParser to be able to "
                           "read the correct unread value from " << qPrintable(path);
    }
}
