#include <iostream>
#include <sstream>

#include "spellchecker.hpp"
/*
 * Note that this include must go last...
 * http://cpputest.github.io/manual.html#memory_leak_detection - see STL
 */
#include "CppUTest/TestHarness.h"

Dictionary* dictionary;
std::string dict_entries("help hell hello loop helps shell helper troop");

TEST_GROUP(SpellCheckerTestGroup)
{
    void setup()
    {
        dictionary = new Dictionary(dict_entries);
    }

    void teardown()
    {
        delete dictionary;
    }
};

TEST(SpellCheckerTestGroup, TestWordExistenceInDict)
{
    CHECK(!dictionary->Exists("foo"));
    dictionary->Add("foo");
    dictionary->Add("fool");
    dictionary->Add("bar");
    CHECK(dictionary->Exists("bar"));
    dictionary->Remove("bar");
    CHECK(!dictionary->Exists("bar"));
}

/*
 * Ensure that all words from the string stream have been added to the dict
 * successfully using the exists api.
 */
TEST(SpellCheckerTestGroup, TestWordsAddedToDictOnInit)
{
    std::string entry;
    std::stringstream ss(dict_entries);
    while (ss >> entry)
    {
        CHECK_TEXT(
            dictionary->Exists(entry),
            ("'" + entry + "' not found!").data());
    }
}

TEST(SpellCheckerTestGroup, TestGetEditDistanceBetweenWords)
{
    CHECK_EQUAL(2, GetEditDistance("hello", "help"));
    CHECK_EQUAL(2, GetEditDistance("shell", "hello"));
    CHECK_EQUAL(5, GetEditDistance("troop", "hello"));
    CHECK_EQUAL(3, GetEditDistance("kitten", "sitting"));
}

/*
 * Check the spelling of a misspelled word should return similar words that are
 * within an edit distance of 2.
 */
TEST(SpellCheckerTestGroup, TestUnkownWordCheckerReturnsSimilar)
{
    CHECK(
        (std::vector<std::string>{"loop", "troop"})
        == dictionary->Check("oop"));
    CHECK(
        (std::vector<std::string>{"loop",})
        == dictionary->Check("lop"));
    CHECK(
        (std::vector<std::string>{"hell", "hello", "help", "shell"})
        == dictionary->Check("sell"));
}
