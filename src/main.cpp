#include <fstream>
#include <iostream>
#include <string>

#include "spellchecker.hpp"
using namespace std;

int main(void)
{
    string s = "";
    while (cin >> s)
    {
        cout << "I got... '" << s << "'" << endl;
    }
    return 0;
}