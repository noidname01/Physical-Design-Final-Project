#ifndef TECH_H
#define TECH_H

#include <cstdio>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>

using namespace std;

class Tech
{
    public:
    class ModuleType
    {
        public:
        vector<vector<int>> _pins; // n * 2
        int _size_x;
        int _size_y;
        int _area;
        int _num_pin;
    };
    vector<ModuleType> _moduleTypes;
    int _num_type;

}; 

#endif  // TECH_H