#include <algorithm>
#include <cassert>
#include <cstring>
#include <sstream>
#include <vector>
#include "spellchecker.hpp"
using namespace std;

int GetEditDistance(string dst, string src)
{
    // https://en.wikipedia.org/wiki/Levenshtein_distance
    // for all i and j, d[i,j] will hold the Levenshtein distance between
    // the first i characters of s and the first j characters of t
    const size_t m = src.size();
    const size_t n = dst.size();
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
            int substitution_cost = (src[i] == dst[j]) ? 0 : 1;
            vector<int> costs{

                distance[i - 1][j] + 1,                        // deletion
                distance[i][j - 1] + 1,                        // insertion
                distance[i - 1][j - 1] + substitution_cost,    // substitution
            };
            distance[i][j] = *min_element(costs.begin(), costs.end());
        }
    }

    return distance[m - 1][n - 1];
}

Entry::~Entry()
{
    // 1. link the parent to each child in target's children...
    for (auto &child : m_children)
    {
        m_parent->LinkTo(child.second);
    }
    // 2. unlink target node from its parent...
    if (m_parent)  // special case for the root node.
    {
        m_parent->Unlink(this);
    }
}

void Entry::Unlink(Entry const *const child)
{
    m_children.erase(GetEditDistanceTo(child->m_word));
}

void Entry::LinkTo(Entry* child)
{
    m_children[GetEditDistanceTo(child->m_word)] = child;
    child->m_parent = this;
}

int Entry::GetEditDistanceTo(string dst) const
{
    return GetEditDistance(dst, m_word);
}

bool Entry::IsLeaf(void) const
{
    return m_children.empty();
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

Dictionary::~Dictionary(void)
{
    if (!m_root)
    {
        return;
    }
    // keep deleting leaf nodes until only the root is left...
    while (!m_root->IsLeaf())
    {
        Entry const *const entry = Search_(
            [](auto entry){return entry->IsLeaf();},
            m_root);
        assert(entry);
        delete entry;
    }
    // then delete the root...
    delete m_root;
}

bool Dictionary::Exists(string word)
{
    if (!m_root)
    {
        return false;
    }
    return Search_(word, m_root) != nullptr;
}

void Dictionary::Add(string word)
{
    if (Exists(word))
    {
        return;
    }
    if (m_root)
    {
        AddToNode_(word, m_root);
    }
    else
    {
        m_root = new Entry(word);
    }
}

void Dictionary::Remove(string word)
{
    if (!Exists(word))
    {
        return;
    }
    Entry const *const to_remove = Search_(word, m_root);
    if (to_remove == m_root)
    {
        m_root = nullptr;
    }
    delete to_remove;
}

void Dictionary::AddToNode_(string word, Entry *const target)
{
    assert(target);
    const int d = target->GetEditDistanceTo(word);
    // if we find an edge the same length,
    if (target->m_children.count(d))
    {
        // recurse to the next node...
        AddToNode_(word, target->m_children[d]);
    }
    else // otherwise add a new node at this edge...
    {
        Entry* new_node = new Entry(word);
        target->LinkTo(new_node);
    }
}

Entry const *const Dictionary::Search_(string &word, Entry const *const root)
{
    return Search_([word](auto entry){return entry->m_word == word;}, root);
}

template <typename MatchingFnType>
Entry const *const Dictionary::Search_(
    MatchingFnType found_entry, Entry const *const root)
{
    // depth first search...
    if (!root)
    {
        return nullptr;
    }
    // return early if the target matches our search...
    if (found_entry(root))
    {
        return root;
    }
    // otherwise search the children...
    // might need to keep track of visited nodes here?
    for (auto &item : root->m_children)
    {
        Entry const *const found = Search_(found_entry, item.second);
        if (found)
        {
            return found;
        }
    }
    // return null if we don't find it...
    return nullptr;
}

vector<string> SpellChecker::Check(string word)
{
    vector<string> result;
    Search_(word, result, m_dict->m_root);
    return result;
}

// recurse into the tree and keep appending to the result each time we find a
// suitable word...
void SpellChecker::Search_(
    const string& word, vector<string>& result, Entry *const node)
{
    if (!node)
    {
        return;
    }
    // 1a. find distance to the node...
    int d = node->GetEditDistanceTo(word);
    // 1b. only add it to the result if it's within tolerance...
    if (d < m_tolerance)
    {
        result.push_back(node->m_word);
    }
    // 2. iterate over nodes within the tolerance limit ie d +/- tolerance...
    for (int idx = (d - m_tolerance); idx < (d + m_tolerance); idx++)
    {
        if (idx < 1)
        {
            // edit distances less than zero don't make sense here.
            continue;
        }
        Search_(word, result, node->m_children[idx]);
    }
}