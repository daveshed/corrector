#include <algorithm>
#include <cassert>
#include <cstring>
#include <sstream>
#include <vector>
#include "spellchecker.hpp"
using namespace std;

int GetEditDistance(string dst, string src)
{
    cout << "Calculating " << dst << " -> " << src << endl;
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
    // 1. link the parent to each child in target's children...
    for (auto &child : m_children)
    {
        // calling this on the root would blow up!
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

void Entry::LinkTo(string word)
{
    const int d = GetEditDistanceTo(word);
    // if we find an edge the same length,
    if (m_children.count(d))
    {
        // recurse to the next node...
        m_children[d]->LinkTo(word);
    }
    else // otherwise add a new node at this edge...
    {
        Entry* new_node = new Entry(word);
        LinkTo(new_node);
    }
}

int Entry::GetEditDistanceTo(string dst) const
{
    return GetEditDistance(dst, m_word);
}

bool Entry::IsLeaf(void) const
{
    return m_children.empty();
}

Entry const *const Entry::Search(string &word) const
{
    return Search([word](auto entry){return entry->GetWord() == word;});
}

template <typename MatchingFnType>
Entry const *const Entry::Search(MatchingFnType found_entry) const
{
    // return early if the target matches our search...
    if (found_entry(this))
    {
        return this;
    }
    // otherwise search the children...
    for (auto &item : m_children)
    {
        Entry const *const found = item.second->Search(found_entry);
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

Dictionary::~Dictionary(void)
{
    if (!m_root)
    {
        return;
    }
    // keep deleting leaf nodes until only the root is left...
    while (!m_root->IsLeaf())
    {
        Entry const *const entry = m_root->Search(
            [](auto entry){return entry->IsLeaf();});
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
    return m_root->Search(word) != nullptr;
}

void Dictionary::Add(string word)
{
    if (Exists(word))
    {
        return;
    }
    if (m_root)
    {
        m_root->LinkTo(word);
    }
    else
    {
        m_root = new RootEntry(word);
    }
}

void Dictionary::Remove(string word)
{
    if (!Exists(word))
    {
        return;
    }
    Entry const *const to_remove = m_root->Search(word);
    if (to_remove == m_root)
    {
        m_root = nullptr;
    }
    delete to_remove;
}

vector<string> Dictionary::Check(string word)
{
    vector<string> result;
    cout << "Checking ..." << word << endl;
    Search_(word, result, m_root);
    cout << "Results ..." << endl;
    sort(result.begin(), result.end());
    for (auto &foo : result)
    {
        cout << foo << endl;
    }
    return result;
}

// recurse into the tree and keep appending to the result each time we find a
// suitable word...
void Dictionary::Search_(
    const string& word, vector<string>& result, Entry const *const node)
{
    cout << "Searching for " << word << " @ " << node->GetWord() << endl;
    if (!node)
    {
        return;
    }
    // 1a. find distance to the node...
    const int d = node->GetEditDistanceTo(word);
    cout << "d = " << d << endl;
    // 1b. only add it to the result if it's within tolerance...
    if (d <= m_tolerance)
    {
        cout << "  Match!" << endl;
        result.push_back(node->GetWord());
    }
    // 2. iterate over nodes within the tolerance limit ie d +/- tolerance...
    cout << "Searching children..." << endl;
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
        cout << "  d = " << idx << endl;
        Search_(
            word,
            result,
            static_cast<Entry const *const>(
                node->GetChildren().find(idx)->second)
        );
    }
}