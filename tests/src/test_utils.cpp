#include <gtest/gtest.h>
#include <utils.h>


using namespace testing;

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

