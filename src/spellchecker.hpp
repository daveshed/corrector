#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

int GetEditDistance(std::string dst, std::string src);
class Dictionary;

// Base class for nodes in the dictionary.
class EntryBase {
public:
    EntryBase(std::string s): m_word(s) {};
    virtual ~EntryBase() {};
    std::string GetWord(void) const {return m_word;};
    std::unordered_map<int, EntryBase*> GetChildren(void) const {
        return m_children;
    };
    void Unlink(EntryBase const *const child);
    void LinkTo(EntryBase* child);
    void LinkTo(std::string word);
    int GetEditDistanceTo(std::string dst) const;
    bool IsLeaf(void) const;
    template <typename MatchingFnType>
    EntryBase const *const Search(MatchingFnType found_entry) const;
    EntryBase const *const Search(std::string &word) const;
protected:  // FIXME -> private
    std::string m_word;
    // parent is stored so that we can remove this node.
    EntryBase* m_parent = nullptr;
    // store a reference to each child and the edit distance to reach it.
    std::unordered_map<int, EntryBase*> m_children;
};

// An entry in the dictionary
struct Entry: public EntryBase {
    Entry(std::string s) : EntryBase(s) {};
    ~Entry();
};

// Entry without a parent - the root of the tree
struct RootEntry : public EntryBase {
friend Dictionary;
public:
    RootEntry(std::string s, Dictionary* d) : EntryBase(s), m_dict(d) {};
    ~RootEntry();
protected:
    Dictionary* m_dict;
};

class Dictionary {
friend RootEntry;
public:
    Dictionary(const std::string& input);
    ~Dictionary();
    bool Exists(std::string word) const;
    bool Empty(void) const;
    void Add(std::string word);
    void Remove(std::string word);
    std::vector<std::string> Check(std::string to_check);
    void Show(EntryBase const* const entry) const;
private:
    void Show_(EntryBase const* const entry, int& depth) const;

private:
    static const int m_tolerance = 2;
    static void Search_(
        const std::string& word,
        std::vector<std::string>& result,
        EntryBase const *const node);

    RootEntry* m_root = nullptr;
};
