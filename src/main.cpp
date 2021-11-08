#include <fstream>
#include <iostream>
#include <string>

#include "spellchecker.hpp"
using namespace std;

int main(void)
{
    Dictionary dict("help hell hello loop helps shell helper troop");
    string tmp;
    while (cin >> tmp)
    {
        cout << endl;
        for (auto corrected : dict.Check(tmp))
        {
            cout << corrected << " ";
        }
    }
    return 0;
}