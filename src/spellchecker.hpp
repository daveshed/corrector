#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

// This is bad practice and should be removed - all those who import will have
// this using declaration.
using namespace std;

int GetEditDistance(string dst, string src);

// An entry in the dictionary
struct Entry {
public:
    Entry(void) {};
    Entry(string s): m_word(s) {};
    ~Entry();
    void Unlink(Entry const *const child);
    void LinkTo(Entry* child);
    int GetEditDistanceTo(string dst) const;
    bool IsLeaf(void) const;

    string m_word;
    // parent is stored so that we can remove this node.
    Entry* m_parent = nullptr;
    // store a reference to each child and the edit distance to reach it.
    unordered_map<int, Entry *> m_children;
};

class Dictionary {
    friend class SpellChecker;
public:
    Dictionary(const string& input);
    ~Dictionary();
    bool Exists(string word);
    void Add(string word);
    void Remove(string word);

private:
    Entry* m_root = nullptr;

private:
    template <typename MatchingFnType>
    static Entry const *const Search_(
        MatchingFnType found_entry, Entry const *const root);
    static Entry const *const Search_(
        string &word, Entry const *const root);
    void AddToNode_(string word, Entry *const target);
};

class SpellChecker {
public:
    SpellChecker(Dictionary const *const dict) : m_dict(dict) {};
    vector<string> Check(string to_check);
private:
    static const int m_tolerance = 2;
    Dictionary const *const m_dict;
    static void Search_(
        const string& word,
        vector<string>& result,
        Entry const *const node);
};
