#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

int GetEditDistance(std::string dst, std::string src);

// An entry in the dictionary
class Entry {
public:
    Entry(std::string s): m_word(s) {};
    ~Entry();
    std::string GetWord(void) const {return m_word;};
    std::unordered_map<int, Entry*> GetChildren(void) const {
        return m_children;
    };
    void Unlink(Entry const *const child);
    void LinkTo(Entry* child);
    void LinkTo(std::string word);
    int GetEditDistanceTo(std::string dst) const;
    bool IsLeaf(void) const;
    template <typename MatchingFnType>
    Entry const *const Search(MatchingFnType found_entry) const;
    Entry const *const Search(std::string &word) const;
protected:  // FIXME -> private
    std::string m_word;
    // parent is stored so that we can remove this node.
    Entry* m_parent = nullptr;
    // store a reference to each child and the edit distance to reach it.
    std::unordered_map<int, Entry*> m_children;
};

// Entry without a parent - the root of the tree
struct RootEntry : public Entry {
public:
    RootEntry(std:: string s) : Entry(s) {};
};


class Dictionary {
public:
    Dictionary(const std::string& input);
    ~Dictionary();
    bool Exists(std::string word);
    void Add(std::string word);
    void Remove(std::string word);
    std::vector<std::string> Check(std::string to_check);

private:
    static const int m_tolerance = 2;
    static void Search_(
        const std::string& word,
        std::vector<std::string>& result,
        Entry const *const node);

    Entry* m_root = nullptr;
};
