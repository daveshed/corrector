#include <fstream>
#include <iostream>
#include <string>

#include "spellchecker.hpp"
using namespace std;

int main(void)
{
    ifstream infile("/home/dmohamad/dev/playground/spellchecker/words.txt");
    Dictionary dict(infile);
    string tmp;
    while (cin >> tmp)
    {
        for (auto corrected : dict.Check(tmp))
        {
            cout << corrected << " ";
        }
        cout << endl;
    }
    return 0;
}