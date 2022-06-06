#include <gtest/gtest.h>
#include <utils.h>


using namespace testing;

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

TEST(UtilsTest, formatGithubMarkdown_mentions) {
    const char* cases[][4] = {
            {"@test",
             "[@test](https://github.com/test)", "@test (https://github.com/test)",
             "Expected formatGithubMarkdown to add a link to the Github account in the mention."},
            {"Some stuff\n@test and more",
             "Some stuff\n[@test](https://github.com/test) and more",
             "Some stuff\n@test (https://github.com/test) and more",
             "Expected formatGithubMarkdown to add a link to the Github account in the mention."},
            {"Some stuff @test and more",
             "Some stuff [@test](https://github.com/test) and more",
             "Some stuff @test (https://github.com/test) and more",
             "Expected formatGithubMarkdown to add a link to the Github account in the mention."},
            {"stuff@test", "stuff@test", "stuff@test",
             "Expected formatGithubMarkdown not to mistake an email address for a mention."},
            {"", "", "", "Expected formatGithubMarkdown to handle empty strings."},
            {"@test-me",
             "[@test-me](https://github.com/test-me)", "@test-me (https://github.com/test-me)",
             "Expected formatGithubMarkdown to add a link to the Github account "
             "if the name contains non-alphabetical letters."},
            {"@test1me",
             "[@test1me](https://github.com/test1me)", "@test1me (https://github.com/test1me)",
             "Expected formatGithubMarkdown to add a link to the Github account "
             "if the name contains non-alphabetical letters."},
            {"(@test1me)",
             "([@test1me](https://github.com/test1me))", "(@test1me (https://github.com/test1me))",
             "Expected formatGithubMarkdown to add a link to the Github account "
             "if the name contains non-alphabetical letters."},
    };
    for (auto &caseData : cases) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const char* expected = caseData[1];
#else
        const char* expected = caseData[2];
#endif
        EXPECT_STREQ(qPrintable(Utils::formatGithubMarkdown(caseData[0])), expected) << caseData[3];
    }
    EXPECT_TRUE(Utils::formatGithubMarkdown(QString()).isNull())
                    << "Expected formatGithubMarkdown to handle null strings.";
}

TEST(UtilsTest, formatGithubMarkdown_links) {
    const char* cases[][2] = {
            {"[text](url)", "text (url)"},
            {"stuff [some text](url?param=1)", "stuff some text (url?param=1)"},
            {"stuff\n[text](url)", "stuff\ntext (url)"},
            {"sun[day](url)more", "sunday (url)more"}, // Not the best, but doesn't need to be
            {"[text] (url)", "[text] (url)"},
            {"([text](url))", "(text (url))"},
    };
    for (auto &caseData : cases) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const char* expected = caseData[0];
        const char* message = "Expected formatGithubMarkdown not to change markdown links.";
#else
        const char* expected = caseData[1];
        const char* message = "Expected formatGithubMarkdown to replace "
                              "markdown links with a textual representation.";
#endif
        EXPECT_STREQ(qPrintable(Utils::formatGithubMarkdown(caseData[0])), expected) << message;
    }
}

TEST(UtilsTest, orderedMapStorage) {
    const int values[][2] = {{1, 2}, {3, 4}, {5, 5}, {6, 7}};
    OrderedMap<int, int> map;
    for (auto &valuePair : values) {
        map[valuePair[0]] = valuePair[1];
    }
    for (auto &valuePair : values) {
        EXPECT_EQ(map[valuePair[0]], valuePair[1])
                        << "Expected the OrderedMap to store the entry with the key "
                        << valuePair[0];
    }
    map[1] = 3;
    EXPECT_EQ(map[1], 3) << "Expected the OrderedMap to update an existing entry";
    const int value = map[1];
    EXPECT_EQ(value, 3) << "Expected the OrderedMap to allow access with the const operator";
}

TEST(UtilsTest, orderedMapOrder) {
    const int values[][2] = {{6, 2}, {3, 4}, {5, 5}, {1, 7}};
    size_t numUniqueValues = ARRAY_SIZE(values);
    OrderedMap<int, int> map;
    for (auto &valuePair : values) {
        map[valuePair[0]] = valuePair[1];
    }
    const QList<int> &orderedKeys = map.orderedKeys();
    EXPECT_EQ(numUniqueValues, (size_t) orderedKeys.size())
                    << "Expected the orderedKeys attribute of the OrderedMap to contain "
                       "the same number of keys as the list of unique key-value pairs";
    for (size_t i = 0; i < numUniqueValues; i++) {
        EXPECT_EQ(values[i][0], orderedKeys[(int) i])
                        << "Expected the keys in the orderedKeys attribute of the "
                           "OrderedMap to be in the order they where inserted.";
    }
    map.remove(3);
    numUniqueValues--;
    EXPECT_EQ(numUniqueValues, (size_t) orderedKeys.size())
                    << "Expected removing an element from the OrderedMap "
                       "to also remove one element from the orderedKeys attribute";
    EXPECT_FALSE(std::any_of(orderedKeys.begin(), orderedKeys.end(),
            [](int element) { return element == 3; }))
                    << "Expected removing an element from the OrderedMap "
                       "to also remove it from the orderedKeys attribute";
    size_t indexBefore = std::find(orderedKeys.begin(), orderedKeys.end(), 6) - orderedKeys.begin();
    map[6] = 1;
    size_t indexAfter = std::find(orderedKeys.begin(), orderedKeys.end(), 6) - orderedKeys.begin();
    EXPECT_EQ(indexBefore, indexAfter)
                    << "Expected overwriting an element from the OrderedMap "
                       "to not change it's index in the orderedKeys attribute";
    EXPECT_EQ(numUniqueValues, (size_t) orderedKeys.size())
                    << "Expected overwriting an element from the OrderedMap "
                       "not to add a new element to the orderedKeys attribute";
}

TEST(UtilsTest, orderedMapClear) {
    const int values[][2] = {{6, 2}, {3, 4}, {5, 5}, {1, 7}};
    OrderedMap<int, int> map;
    for (auto &valuePair : values) {
        map[valuePair[0]] = valuePair[1];
    }
    EXPECT_FALSE(map.isEmpty())
                    << "Expected the OrderedMap not to be empty after adding some values";
    EXPECT_FALSE(map.orderedKeys().isEmpty())
                    << "Expected the the orderedKeys attribute of the OrderedMap "
                       "not to be empty after adding some values to the OrderedMap";
    map.clear();
    EXPECT_TRUE(map.isEmpty()) << "Expected the OrderedMap to be empty after being cleared";
    EXPECT_TRUE(map.orderedKeys().isEmpty())
                    << "Expected the the orderedKeys attribute of the OrderedMap "
                       "to be empty after the OrderedMap cleared";
}
