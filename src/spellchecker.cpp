#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "spellchecker.hpp"
using namespace std;

// FIXME - move to private header
#define DEBUG
#ifdef DEBUG
    #define DEBUG_LOG(x)  (cout << x << endl)
#else
    #define DEBUG_LOG(x)
#endif

int GetEditDistance(string dst, string src)
{
    // https://en.wikipedia.org/wiki/Levenshtein_distance
    // for all i and j, d[i,j] will hold the Levenshtein distance between
    // the first i characters of src and the first j characters of dst
    const size_t m = src.size() + 1;
    const size_t n = dst.size() + 1;
    int distance[m][n];

    // set each element in d to zero
    memset(distance, 0, sizeof(distance[0][0]) * m * n);

    // source prefixes can be transformed into empty string by
    // dropping all characters
    for (int i = 1; i < m; i++)
    {
        distance[i][0] = i;
    }
    // destination prefixes can be reached from empty source prefix
    // by inserting every character
    for (int j = 1; j < n; j++)
    {
        distance[0][j] = j;
    }

    for (int j = 1; j < n; j++)
    {
        for (int i = 1; i < m; i++)
        {
            int substitution_cost = (src[i - 1] == dst[j - 1]) ? 0 : 1;
            vector<int> costs{
                distance[i - 1][j] + 1,                        // deletion
                distance[i][j - 1] + 1,                        // insertion
                distance[i - 1][j - 1] + substitution_cost,    // substitution
            };
            distance[i][j] = *min_element(costs.begin(), costs.end());
        }
    }

    #if 0
    // print out the array...
    cout << "   ";
    for (auto &c : dst)
    {
        cout << " " << c;
    }
    cout << endl;
    for (int i = 0; i < m; i++)  // src
    {
        char foo = (i == 0) ? ' ' : src[i - 1];
        cout << foo << " ";
        for (int j = 0; j < n; j++)  // src
        {
            cout << distance[i][j] << " ";
        }
        cout << endl;
    }
    #endif
    return distance[m - 1][n - 1];
}

Entry::~Entry()
{
    DEBUG_LOG("Entry deconstructor: " + GetWord());
    // Link the parent to each child in target's children...
    for (auto &child : m_children)
    {
        // calling this on the root would blow up!
        m_parent->LinkTo(child.second);
    }
    m_parent->Unlink(this);
}

RootEntry::~RootEntry()
{
    DEBUG_LOG("Root deconstructor: " << GetWord());
    // (A) Root is the only node
    if (IsLeaf())
    {
        // nothing to do - you can just delete me and update the dict
        DEBUG_LOG("  Leaf");
        m_dict->m_root = nullptr;
        return;
    }
    // (B) Root has children
    //  1. Pick the first child as the new root node...
    // FIXME: Is the first one really the best one? would the smallest edit
    //        distance actually be better?
    EntryBase* to_promote = m_children.begin()->second;
    RootEntry* new_root = new RootEntry(
        to_promote->GetWord(),
        m_dict);
    DEBUG_LOG("  New root is " + new_root->GetWord());
    //  2. Link the new root to the other children...
    for (auto it = ++m_children.begin(); it != m_children.end(); ++it)
    {
        DEBUG_LOG("  Linking to " << it->second->GetWord());
        // This is overwriting keys in the mapping!!
        // Should we rebuild the dictionary?
        new_root->LinkTo(it->second);
    }
    //  3. update the dictionary to hold the new root...
    m_dict->m_root = new_root;
    //  4. Get rid of the duplicated node...
    delete to_promote;
}

void EntryBase::Unlink(EntryBase const *const child)
{
    m_children.erase(GetEditDistanceTo(child->m_word));
}

// Link to a node that already exists
void EntryBase::LinkTo(EntryBase* target)
{
    assert(target);
    const int d = GetEditDistanceTo(target->m_word);
    // do not overwrite the keys - recurse if needed.
    if (m_children.count(d))
    {
        // recurse...
        m_children[d]->LinkTo(target);
    }
    else
    {
        m_children[d] = target;
        target->m_parent = this;
    }
}

// Create a node for the new word and link it
void EntryBase::LinkTo(string word)
{
    Entry* new_node = new Entry(word);
    LinkTo(new_node);
}

int EntryBase::GetEditDistanceTo(string dst) const
{
    return GetEditDistance(dst, m_word);
}

bool EntryBase::IsLeaf(void) const
{
    return m_children.empty();
}

EntryBase const *const EntryBase::Search(string &word) const
{
    return Search([word](auto entry){return entry->GetWord() == word;});
}

template <typename MatchingFnType>
EntryBase const *const EntryBase::Search(MatchingFnType found_entry) const
{
    // return early if the target matches our search...
    if (found_entry(this))
    {
        return this;
    }
    // otherwise search the children...
    for (auto &item : m_children)
    {
        EntryBase const *const found = item.second->Search(found_entry);
        if (found)
        {
            return found;
        }
    }
    // return null if we don't find it...
    return nullptr;
}

Dictionary::Dictionary(const string& input)
{
    stringstream ss(input);
    string entry;
    while (ss >> entry)
    {
        Add(entry);
    }
}

Dictionary::Dictionary(ifstream& input)
{
    string entry;
    while (input >> entry)
    {
        Add(entry);
    }
    DEBUG_LOG("Initialised");
}

Dictionary::~Dictionary(void)
{
    // keep deleting leaf nodes until there are no more...
    DEBUG_LOG("Dictionary is deleting entries...");
    while (!Empty())
    {
        EntryBase const *const entry = m_root->Search(
            [](auto entry){return entry->IsLeaf();});
        DEBUG_LOG("Deleting: " << entry->GetWord());
        assert(entry);
        delete entry;
    }
}

bool Dictionary::Exists(string word) const
{
    if (!m_root)
    {
        return false;
    }
    return m_root->Search(word) != nullptr;
}

bool Dictionary::Empty(void) const
{
    return m_root == nullptr;
}

void Dictionary::Add(string word)
{
    if (Exists(word))
    {
        return;
    }
    if (Empty())
    {
        m_root = new RootEntry(word, this);
    }
    else
    {
        m_root->LinkTo(word);
    }
}

void Dictionary::Remove(string word)
{
    DEBUG_LOG("Before removal...");
    Show(m_root);
    if (!Exists(word))
    {
        return;
    }
    delete m_root->Search(word);
    DEBUG_LOG("After removal...");
    Show(m_root);
}

vector<string> Dictionary::Check(string word)
{
    vector<string> result;
    DEBUG_LOG("Checking ..." + word);
    Search_(word, result, m_root);
    if (std::find(result.begin(), result.end(), word) != result.end())
    {
        // The word exists in the dictionary. It's correct so return nothing.
        result.clear();
        return result;
    }
    sort(result.begin(), result.end());
    DEBUG_LOG("Results ...");
    for (auto &candidate : result)
    {
        DEBUG_LOG(candidate);
    }
    return result;
}

// recurse into the tree and keep appending to the result each time we find a
// suitable word...
void Dictionary::Search_(
    const string& word, vector<string>& result, EntryBase const *const node)
{
    DEBUG_LOG("Searching for " + word + " @ " + node->GetWord());
    if (!node)
    {
        return;
    }
    // 1a. find distance to the node...
    const int d = node->GetEditDistanceTo(word);
    // 1b. only add it to the result if it's within tolerance...
    if (d <= m_tolerance)
    {
        result.push_back(node->GetWord());
    }
    // 2. iterate over nodes within the tolerance limit ie d +/- tolerance...
    for (int idx = (d - m_tolerance); idx < (d + m_tolerance); idx++)
    {
        if (idx < 1)
        {
            // edit distances less than zero don't make sense here.
            continue;
        }
        if (!node->GetChildren().count(idx))
        {
            continue;
        }
        Search_(
            word,
            result,
            static_cast<Entry const *const>(
                node->GetChildren().find(idx)->second)
        );
    }
}

// breadth first recursive search that displays the contents to stdout.
void Dictionary::Show(EntryBase const* const entry) const
{
    int depth = 0;
    Show_(m_root, depth);
}

// need to remember the depth so we can pad the correct number of spaces.
// pass a reference to an int?
void Dictionary::Show_(EntryBase const* const entry, int& depth) const
{
    std::string padding(depth * 2, ' ');
    DEBUG_LOG(padding << entry->GetWord());
    if (entry->IsLeaf())
    {
        return;
    }
    depth += 1;
    for (auto &item : entry->GetChildren())
    {
        Show_(item.second, depth);
    }
}
