#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

int GetEditDistance(std::string dst, std::string src);

// An entry in the dictionary
struct Entry {
public:
    Entry(void) {};
    Entry(std::string s): m_word(s) {};
    ~Entry();
    void Unlink(Entry const *const child);
    void LinkTo(Entry* child);
    int GetEditDistanceTo(std::string dst) const;
    bool IsLeaf(void) const;

    std::string m_word;
    // parent is stored so that we can remove this node.
    Entry* m_parent = nullptr;
    // store a reference to each child and the edit distance to reach it.
    std::unordered_map<int, Entry *> m_children;
};

class Dictionary {
    friend class SpellChecker;
public:
    Dictionary(const std::string& input);
    ~Dictionary();
    bool Exists(std::string word);
    void Add(std::string word);
    void Remove(std::string word);

private:
    Entry* m_root = nullptr;

private:
    template <typename MatchingFnType>
    static Entry const *const Search_(
        MatchingFnType found_entry, Entry const *const root);
    static Entry const *const Search_(
        std::string &word, Entry const *const root);
    void AddToNode_(std::string word, Entry *const target);
};

class SpellChecker {
public:
    SpellChecker(Dictionary const *const dict) : m_dict(dict) {};
    std::vector<std::string> Check(std::string to_check);
private:
    static const int m_tolerance = 2;
    Dictionary const *const m_dict;
    static void Search_(
        const std::string& word,
        std::vector<std::string>& result,
        Entry const *const node);
};
