#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <map>
#include <algorithm>
#include <queue>
#include "place.h"

using namespace std;

void Placer::parseInput(fstream& inFile)
{
    string str;
    // Num of techs
    inFile >> str >> str;
    _num_techs = stod(str);
    _techs.resize(_num_techs);

    for(int i = 0 ; i < _num_techs; ++i){
        inFile >> str >> str;
        _techNameToId[str] = i;
        inFile >> str;
        int num_type = stod(str);
        _techs[i]._num_type = num_type;
        _techs[i]._moduleTypes.resize(num_type);
        for(int j = 0; j < num_type; ++j){
            //LibCell MCX
            inFile >> str >> str;;
            _typeNameToId[str] = j;

            inFile >> str;
            int itemp = stod(str);
            _techs[i]._moduleTypes[j]._size_x = itemp;

            inFile >> str;
            itemp = stod(str);
            _techs[i]._moduleTypes[j]._size_y = itemp;
            _techs[i]._moduleTypes[j]._area = _techs[i]._moduleTypes[j]._size_x * _techs[i]._moduleTypes[j]._size_y;

            inFile >> str;
            itemp = stod(str);
            _techs[i]._moduleTypes[j]._num_pin = itemp;
            _techs[i]._moduleTypes[j]._pins.resize(itemp);

            for(int k = 0; k < itemp; ++k){
                //Pin PX
                inFile >> str >> str;
                if(_pinNameToId.count(str) == 0){
                    _pinNameToId[str] = k;
                }
                inFile >> str;
                int off = stod(str);
                // off x : _pin[k][0]
                _techs[i]._moduleTypes[j]._pins[k].push_back(off);
                inFile >> str;
                off = stod(str);
                // off y : _pin[k][1]
                _techs[i]._moduleTypes[j]._pins[k].push_back(off);
            }

        }

    }

    // DieSize 
    inFile >> str;
    inFile >> str;
    _dieLLX = stod(str);
    inFile >> str;
    _dieLLY = stod(str);
    inFile >> str;
    _dieURX = stod(str);
    inFile >> str;
    _dieURY = stod(str);
    _dieArea = (_dieURX - _dieLLX) * (_dieURY - _dieLLY);

    // TopDieMaxUtil
    inFile >> str;
    inFile >> str;
    _topMaxUtil = stod(str);
    // BottomDieMaxUtil
    inFile >> str;
    inFile >> str;
    _botMaxUtil = stod(str);

    // TopDieRows
    inFile >> str;
    inFile >> str;
    _topRowLocX = stod(str);
    inFile >> str;
    _topRowLocY = stod(str);
    inFile >> str;
    _topRowWidth = stod(str);
    inFile >> str;
    _topRowHeight = stod(str);
    inFile >> str;
    _num_topRow = stod(str);

    // BottomDieRows
    inFile >> str;
    inFile >> str;
    _botRowLocX = stod(str);
    inFile >> str;
    _botRowLocY = stod(str);
    inFile >> str;
    _botRowWidth = stod(str);
    inFile >> str;
    _botRowHeight = stod(str);
    inFile >> str;
    _num_botRow = stod(str);

    // TopDieTech
    inFile >> str;
    inFile >> str;
    _topTech = _techNameToId[str];

    // BottomDieTech
    inFile >> str;
    inFile >> str;
    _botTech = _techNameToId[str];

    // TerminalSize
    inFile >> str;
    inFile >> str;
    _sizeTerminalX = stod(str);
    inFile >> str;
    _sizeTerminalY = stod(str);

    // TerminalSpacing
    inFile >> str;
    inFile >> str;
    _spacingTerminal = stod(str);

    // NumInstances
    inFile >> str;
    inFile >> str;
    _num_module = stod(str);

    bool topOverUtil = false;
    bool botOverUtil = false;
    int topCurrentArea = 0;
    int botCurrentArea = 0;
    double topUtil = 0;
    double botUtil = 0;

    for(int i = 0; i < _num_module; ++i){
        // Inst
        inFile >> str;
        string moduleName;
        inFile >> moduleName;
        _moduleNameToId[moduleName] = i;
        string typeName;
        inFile >> typeName;
        _moduleIdToType.push_back(_typeNameToId[typeName]);
        // initial partition
        // balance 
        if(i < (_num_module/2) && !(topOverUtil)){
            int tech = _topTech;
            int m_width = _techs[tech]._moduleTypes[_typeNameToId[typeName]]._size_x;
            int m_height = _techs[tech]._moduleTypes[_typeNameToId[typeName]]._size_y;
            int m_area = m_width * m_height; 
            topCurrentArea += m_area;
            double tempUtil = topUtil;
            topUtil = (double)topCurrentArea / (double)_dieArea;
            if(topUtil > ((double)_topMaxUtil / 100.0)){
                topOverUtil = true;   
                topUtil = tempUtil;
                topCurrentArea -= m_area;
                tech = _botTech;
                m_width = _techs[tech]._moduleTypes[_typeNameToId[typeName]]._size_x;
                m_height = _techs[tech]._moduleTypes[_typeNameToId[typeName]]._size_y;
                m_area = m_width * m_height; 
                botCurrentArea += m_area;
                double tempUtil = botUtil;
                botUtil = (double)botCurrentArea / (double)_dieArea;
                _modules.push_back(Module(moduleName, 1, tech, _typeNameToId[typeName], m_width, m_height));
            }
            else{
                _modules.push_back(Module(moduleName, 0, tech, _typeNameToId[typeName], m_width, m_height));
            }
            if(topOverUtil){
                cout << "Top over!!" << endl;
                cout << "top used area : " << topCurrentArea << endl;
                cout << "Die area : " << _dieArea << endl;
                cout << "top used area ratio : " << (double)topCurrentArea / (double)_dieArea << endl;
                cout << "top Util : " << topUtil << endl;
            }
        }
        else if(!(botOverUtil)){
            int tech = _botTech;
            int m_width = _techs[tech]._moduleTypes[_typeNameToId[typeName]]._size_x;
            int m_height = _techs[tech]._moduleTypes[_typeNameToId[typeName]]._size_y;
            int m_area = m_width * m_height; 
            botCurrentArea += m_area;
            double tempUtil = botUtil;
            botUtil = (double)botCurrentArea / (double)_dieArea;
            if(botUtil > ((double)_botMaxUtil / 100.0)){
                botOverUtil = true;
                botUtil = tempUtil;
                botCurrentArea -= m_area;
                tech = _topTech;
                m_width = _techs[tech]._moduleTypes[_typeNameToId[typeName]]._size_x;
                m_height = _techs[tech]._moduleTypes[_typeNameToId[typeName]]._size_y;
                m_area = m_width * m_height; 
                topCurrentArea += m_area;
                double tempUtil = topUtil;
                topUtil = (double)topCurrentArea / (double)_dieArea;
                _modules.push_back(Module(moduleName, 0, tech, _typeNameToId[typeName], m_width, m_height));
            }
            else{
                _modules.push_back(Module(moduleName, 1, tech, _typeNameToId[typeName], m_width, m_height));
            }
            if(botOverUtil){
                cout << "Bot over!!" << endl;
                cout << "Bot used area : " << botCurrentArea << endl;
                cout << "Die area : " << _dieArea << endl;
                cout << "Bot used area ratio : " << (double)botCurrentArea / (double)_dieArea << endl;
                cout << "Bot Util : " << botUtil << endl;
            }
        }
        else{
            int tech = _topTech;
            int m_width = _techs[tech]._moduleTypes[_typeNameToId[typeName]]._size_x;
            int m_height = _techs[tech]._moduleTypes[_typeNameToId[typeName]]._size_y;
            int m_area = m_width * m_height; 
            topCurrentArea += m_area;
            double tempUtil = topUtil;
            topUtil = (double)topCurrentArea / (double)_dieArea;
            _modules.push_back(Module(moduleName, 0, tech, _typeNameToId[typeName], m_width, m_height));
        }
        
    }

    cout << endl << "After initial partition." << endl << endl;

    cout << "top used area : " << topCurrentArea << endl;
    cout << "Die area : " << _dieArea << endl;
    cout << "top used area ratio : " << (double)topCurrentArea / (double)_dieArea << endl;
    cout << "top Util : " << topUtil << endl;

    cout << "Bot used area : " << botCurrentArea << endl;
    cout << "Die area : " << _dieArea << endl;
    cout << "Bot used area ratio : " << (double)botCurrentArea / (double)_dieArea << endl;
    cout << "Bot Util : " << botUtil << endl;

    // NumNets
    inFile >> str;
    inFile >> str;
    _num_net = stod(str);

    for(int i = 0; i < _num_net; ++i){
        // Net
        inFile >> str;
        string netName;
        inFile >> netName;
        if(_netNameToId.count(netName) == 0){
            _netNameToId[netName] = i;
        }
        inFile >> str;
        int num_pin = stod(str);
        vector<vector<int>> net;
        for(int j = 0; j < num_pin; ++j){
            // Pin
            inFile >> str;
            inFile >> str;
            size_t pos = str.find("/");
            string m_name = str.substr(0, pos);
            string p_name = str.substr(pos+1);
            int mId = _moduleNameToId[m_name];
            int pId = _pinNameToId[p_name];
            vector<int> pin;
            pin.push_back(mId);
            pin.push_back(pId);
            net.push_back(pin);
        }
        _nets.push_back(net);
        
    }
    return;
}

void Placer::initialPartition(){
    vector<int> m_top_list;
    vector<int> m_bot_list;
    vector<vector<int>> n_list;
    for(int i = 0; i < _num_module; ++i){
        if(_modules[i].m_die == 0){
            m_top_list.push_back(i);
        }
        else{
            m_bot_list.push_back(i);
        }
    }
    for(int i = 0; i < _num_net; ++i){
        vector<int> modules;
        for(int j = 0; j < _nets[i].size(); ++j){
            modules.push_back(_nets[i][j][0]);
        }
        n_list.push_back(modules);
    }
    for(int i = 0; i < n_list.size(); ++i){
        //cout << i << " net : " << endl;
        for(int j = 0; j < n_list[i].size(); ++j){
            //cout << n_list[i][j] << endl;
        }
    }

    cout << "top : " << m_top_list.size() << endl;
    cout << "bot : " << m_bot_list.size() << endl;

    cout << endl << "Start FM." << endl << endl;

    _partitioner = new Partitioner(_dieArea, _topMaxUtil, _botMaxUtil, m_top_list, m_bot_list, n_list, _techs, _topTech, _botTech, _moduleIdToType);
    _partitioner->partition();
    _partitioner->printSummary();

}

void Placer::outputPartitionResult(string& fileName){

    fstream fout;
    fstream fout2;
    // top scl
    fout.open(fileName + "_top.scl", ios::out);
    fout << "UCLA scl 1.0" << endl << endl << endl;
    fout << "Numrows : " << _num_topRow << endl << endl;
    
    for(int i = 0; i < _num_topRow; ++i){
        fout << "CoreRow Horizontal" << endl;
        fout << " Coordinate   :\t" << _topRowLocY + i * _topRowHeight << endl;
        fout << " Height       :\t" << _topRowHeight << endl;
        fout << " Sitewidth    :\t" << 1 << endl;
        fout << " Sitespacing  :\t" << 1 << endl;
        fout << " Siteorient   :\t" << "N" << endl;
        fout << " Sitesymmetry :\t" << "Y" << endl;
        fout << " SubrowOrigin :\t" << _topRowLocX;
        fout << " Numsites :\t" << _topRowWidth << endl;
        fout << "End" << endl;
    }

    fout.close();

    // bot scl
    fout.open(fileName + "_bot.scl", ios::out);
    fout << "UCLA scl 1.0" << endl << endl << endl;
    fout << "Numrows : " << _num_botRow << endl << endl;
    
    for(int i = 0; i < _num_botRow; ++i){
        fout << "CoreRow Horizontal" << endl;
        fout << " Coordinate   :\t" << _botRowLocY + i * _botRowHeight << endl;
        fout << " Height       :\t" << _botRowHeight << endl;
        fout << " Sitewidth    :\t" << 1 << endl;
        fout << " Sitespacing  :\t" << 1 << endl;
        fout << " Siteorient   :\t" << "N" << endl;
        fout << " Sitesymmetry :\t" << "Y" << endl;
        fout << " SubrowOrigin :\t" << _botRowLocX;
        fout << " Numsites :\t" << _botRowWidth << endl;
        fout << "End" << endl;
    }

    fout.close();

    // top/bot pl
    fout.open(fileName + "_top.pl", ios::out);
    fout2.open(fileName + "_bot.pl", ios::out);
    fout << "UCLA pl 1.0" << endl << endl << endl;
    fout2 << "UCLA pl 1.0" << endl << endl << endl;
    for(int i = 0; i < _num_module; ++i){
        if(_partitioner->_cellArray[i]->getPart() == 0){
            fout << _modules[i].m_name << "\t" << _dieURX/2 << "\t" << _dieURY/2 << "\t" << 0 << " : N" << endl;
        }
        else{
            fout2 << _modules[i].m_name << "\t" << _dieURX/2 << "\t" << _dieURY/2 << " : N" << endl;
        }
    }

    fout.close();
    fout2.close();

    // top/bot wts
    fout.open(fileName + "_top.wts", ios::out);
    fout2.open(fileName + "_bot.wts", ios::out);
    fout << "UCLA wts 1.0" << endl << endl << endl;
    fout2 << "UCLA wts 1.0" << endl << endl << endl;
    for(int i = 0; i < _num_module; ++i){
        if(_partitioner->_cellArray[i]->getPart() == 0){
            fout << _modules[i].m_name << "\t" << 1 << endl;
        }
        else{
            fout2 << _modules[i].m_name << "\t" << 1 << endl;
        }
    }

    fout.close();
    fout2.close();

    // top/bot nodes
    fout.open(fileName + "_top.nodes", ios::out);
    fout2.open(fileName + "_bot.nodes", ios::out);
    fout << "UCLA nodes 1.0" << endl << endl << endl;
    fout2 << "UCLA nodes 1.0" << endl << endl << endl;
    fout << "NumNodes :\t" << _partitioner->getPartSize(0) << endl;
    fout2 << "NumNodes :\t" << _partitioner->getPartSize(1) << endl;
    fout << "NumTerminals :\t" << 0 << endl << endl;
    fout2 << "NumTerminals :\t" << 0 << endl << endl;
    for(int i = 0; i < _num_module; ++i){
        if(_partitioner->_cellArray[i]->getPart() == 0){
            fout << _modules[i].m_name << "\t" << _techs[_topTech]._moduleTypes[_moduleIdToType[i]]._size_x << "\t";
            fout << _techs[_topTech]._moduleTypes[_moduleIdToType[i]]._size_y << endl;
        }
        else{
            fout2 << _modules[i].m_name << "\t" << _techs[_botTech]._moduleTypes[_moduleIdToType[i]]._size_x << "\t";
            fout2 << _techs[_botTech]._moduleTypes[_moduleIdToType[i]]._size_y << endl;
        }
    }

    fout.close();
    fout2.close();

    // top/bot nets
    fout.open(fileName + "_top.nets", ios::out);
    fout2.open(fileName + "_bot.nets", ios::out);
    fout << "UCLA nets 1.0" << endl << endl << endl;
    fout2 << "UCLA nets 1.0" << endl << endl << endl;
    int num_top_pin = 0;
    int num_bot_pin = 0;
    int num_top_net = 0;
    int num_bot_net = 0;
    string topNetInfo;
    string botNetInfo;
    for(int i = 0; i < _partitioner->getNetNum(); ++i){
        Net_p* currentNet = _partitioner->_netArray[i];
        vector<int> currentNetCellList = currentNet->getCellList();
        if(currentNet->getPartCount(0) != 0){
            ++num_top_net;
        }
        if(currentNet->getPartCount(1) != 0){
            ++num_bot_net;
        }
        int top_net_degree = 0;
        int bot_net_degree = 0;
        for(int j = 0; j < currentNetCellList.size(); ++j){
            int currentCellId = currentNetCellList[j];
            Cell* currentCell = _partitioner->_cellArray[currentNetCellList[j]];
            if(currentCell->getPart() == 0){
                ++top_net_degree;
            }
            if(currentCell->getPart() == 1){
                ++bot_net_degree;
            }
        }
        num_top_pin += top_net_degree;
        num_bot_pin += bot_net_degree;
    }
    
    fout << "NumNets :\t" << num_top_net << endl;
    fout2 << "NumNets :\t" << num_bot_net << endl;
    fout << "NumPins :\t" << num_top_pin << endl << endl;;
    fout2 << "NumPins :\t" << num_bot_pin << endl << endl;;

    // write nets
    for(int i = 0; i < _partitioner->getNetNum(); ++i){
        topNetInfo.clear();
        botNetInfo.clear();
        Net_p* currentNet = _partitioner->_netArray[i];
        vector<int> currentNetCellList = currentNet->getCellList();
        int top_net_degree = 0;
        int bot_net_degree = 0;
        for(int j = 0; j < currentNetCellList.size(); ++j){
            int currentCellId = currentNetCellList[j];
            Cell* currentCell = _partitioner->_cellArray[currentNetCellList[j]];
            if(currentCell->getPart() == 0){
                ++top_net_degree;
                topNetInfo.append("\t");
                topNetInfo.append(_modules[currentCellId].GetName());
                topNetInfo.append("\tI : ");
                double offset_x = (double)_techs[_topTech]._moduleTypes[_moduleIdToType[currentCellId]]._size_x / 2.0;
                double offset_y = (double)_techs[_topTech]._moduleTypes[_moduleIdToType[currentCellId]]._size_y / 2.0;
                topNetInfo.append(to_string(_techs[_topTech]._moduleTypes[_moduleIdToType[currentCellId]]._pins[_nets[i][j][1]][0] - offset_x));
                topNetInfo.append(" ");
                topNetInfo.append(to_string(_techs[_topTech]._moduleTypes[_moduleIdToType[currentCellId]]._pins[_nets[i][j][1]][1] - offset_y));
                topNetInfo.append("\n");
            }
            if(currentCell->getPart() == 1){
                ++bot_net_degree;
                botNetInfo.append("\t");
                botNetInfo.append(_modules[currentCellId].GetName());
                botNetInfo.append("\tI : ");
                double offset_x = (double)_techs[_botTech]._moduleTypes[_moduleIdToType[currentCellId]]._size_x / 2.0;
                double offset_y = (double)_techs[_botTech]._moduleTypes[_moduleIdToType[currentCellId]]._size_y / 2.0;
                botNetInfo.append(to_string(_techs[_botTech]._moduleTypes[_moduleIdToType[currentCellId]]._pins[_nets[i][j][1]][0] - offset_x));
                botNetInfo.append(" ");
                botNetInfo.append(to_string(_techs[_botTech]._moduleTypes[_moduleIdToType[currentCellId]]._pins[_nets[i][j][1]][1] - offset_y));
                botNetInfo.append("\n");
            }
        }
        if(top_net_degree){
            fout << "NetDegree :\t" << top_net_degree << endl;
            fout << topNetInfo;
        }
        if(bot_net_degree){
            fout2 << "NetDegree :\t" << bot_net_degree << endl;
            fout2 << botNetInfo;
        }
    }

    fout.close();
    fout2.close();

    // top/bot aux
    size_t pos = fileName.rfind("/");
    string caseName;
    if(pos != string::npos){
        caseName = fileName.substr(pos+1);
    }
    else{
        caseName = fileName;
    }
    fout.open(fileName + "_top.aux", ios::out);
    fout2.open(fileName + "_bot.aux", ios::out);
    fout << "RowBasedPlacement : ";
    fout2 << "RowBasedPlacement : ";
    string topAux;
    string botAux;
    topAux.append(caseName + "_top.nodes " + caseName + "_top.nets " + caseName + "_top.wts " + caseName + "_top.pl " + caseName + "_top.scl");
    botAux.append(caseName + "_bot.nodes " + caseName + "_bot.nets " + caseName + "_bot.wts " + caseName + "_bot.pl " + caseName + "_bot.scl");
    fout << topAux;
    fout2 << botAux;
    
    fout.close();
    fout2.close();


}

void Placer::BFS_via(int currentNetId,int i_x, int i_y){
    queue<vector<int>> QQ;
    vector<vector<bool>> colorMap;
    colorMap.resize(_num_via_x);
    for(int i = 0; i < _num_via_x; ++i){
        colorMap[i].resize(_num_via_y, 0);
    }
    colorMap[i_x][i_y] = 1;
    QQ.push({i_x, i_y});
    while (!QQ.empty())
    {
        vector<int> via = QQ.front();
        QQ.pop();
        if(!_occupiedViaMap[via[0]][via[1]]){
            _occupiedViaMap[via[0]][via[1]] = 1;
            _cuttedNetIdToNearestVia[currentNetId] = {via[0], via[1]};
            return;
        }
        vector<int> up = {i_x, min(i_y+1, _num_via_y-1)};
        vector<int> down = {i_x, max(i_y-1, 0)};
        vector<int> left = {max(i_x-1, 0), i_y};
        vector<int> right = {min(i_x+1, _num_via_x-1), i_y};
        vector<vector<int>> adj = {up, down, left, right};
        for(int i = 0; i < 4; ++i){
            if(colorMap[adj[i][0]][adj[i][1]] == 0){
                colorMap[adj[i][0]][adj[i][1]] = 1;
                QQ.push(adj[i]);
            }
        }
    }
}

void Placer::viaDecision(string& fileName){
    bool die = !(_partitioner->getPartSize(0) <= _partitioner->getPartSize(1));
    string postfix = (_partitioner->getPartSize(0) <= _partitioner->getPartSize(1))? "_top.gp.pl" : "_bot.gp.pl";
    fstream plFile;
    string plFileName = fileName + postfix;
    plFile.open(plFileName, ios::in);
    if(!plFile){
        cout << endl << ".gp.pl file does not exist!!" << endl<< endl;
        return;
    }
    
    // read .gp.pl
    string str;
    plFile >> str >> str >> str;
    for(int i = 0; i < _partitioner->getPartSize(die); ++i){
        plFile >> str;
        int mId = _moduleNameToId[str];
        plFile >> str;
        int mX = stoi(str);
        plFile >> str;
        int mY = stoi(str);
        _modules[mId].m_x = mX;
        _modules[mId].m_y = mY;
        _modules[mId].CalcCenter();
        plFile >> str >> str;
    }

    plFile.close();

    _netIdToIsCutted.resize(_num_net, false);

    for(int i = 0; i < _partitioner->getNetNum(); ++i){
        Net_p* currentNet = _partitioner->_netArray[i];
        vector<int> currentNetCellList = currentNet->getCellList();
        // cutted net condition
        if(currentNet->getPartCount(0) && currentNet->getPartCount(1)){
            _netIdToIsCutted[i] = true;
            _cutted_net.push_back(i);
            double num_pin = 0;
            double posX = 0;
            double posY = 0;
            double maxX = -INT32_MAX;
            double maxY = -INT32_MAX;
            double minX = INT32_MAX;
            double minY = INT32_MAX;
            // vector : CM location
            vector<double> CM;
            for(int j = 0; j < currentNetCellList.size(); ++j){
                int currentCellId = currentNetCellList[j];
                Cell* currentCell = _partitioner->_cellArray[currentNetCellList[j]];
                if(currentCell->getPart() == 0 && die == 0){
                    // pin position
                    double pin_x = _techs[_topTech]._moduleTypes[_moduleIdToType[currentCellId]]._pins[_nets[i][j][1]][0];
                    double pin_y = _techs[_topTech]._moduleTypes[_moduleIdToType[currentCellId]]._pins[_nets[i][j][1]][1];
                    posX += pin_x + _modules[currentCellId].m_x;
                    posY += pin_y + _modules[currentCellId].m_y;
                    maxX = max(pin_x, maxX);
                    maxY = max(pin_y, maxY);
                    minX = min(pin_x, minX);
                    minY = min(pin_y, minY);
                    ++num_pin;
                }
                if(currentCell->getPart() == 1 && die == 1){
                    // pin position
                    double pin_x = _techs[_botTech]._moduleTypes[_moduleIdToType[currentCellId]]._pins[_nets[i][j][1]][0];
                    double pin_y = _techs[_botTech]._moduleTypes[_moduleIdToType[currentCellId]]._pins[_nets[i][j][1]][1];
                    posX += pin_x + _modules[currentCellId].m_x;
                    posY += pin_y + _modules[currentCellId].m_y;
                    maxX = max(pin_x, maxX);
                    maxY = max(pin_y, maxY);
                    minX = min(pin_x, minX);
                    minY = min(pin_y, minY);
                    ++num_pin;
                }
                
            }
            posX /= num_pin;
            posY /= num_pin;
            CM.push_back(posX);
            CM.push_back(posY);
            _cuttedNetIdToCM[i] = CM;
            _cuttedNetIdToHPWL[i] = (maxX - minX) + (maxY - minY);
        }
        
    }

    sort(_cutted_net.begin(), _cutted_net.end(), [this](int a, int b){
        return _cuttedNetIdToHPWL[a] < _cuttedNetIdToHPWL[b]; // ascending
    });
    

    // calculate via coordinates 
    _num_via_x = (_dieURX - _spacingTerminal) / (_sizeTerminalX + _spacingTerminal);
    _num_via_y = (_dieURY - _spacingTerminal) / (_sizeTerminalY + _spacingTerminal);
    _occupiedViaMap.resize(_num_via_x);
    for(int i = 0; i < _num_via_x; ++i){
        _occupiedViaMap[i].resize(_num_via_y, false);
    }
    cout << "num via x : " << _num_via_x << endl;
    cout << "num via y : " << _num_via_y << endl;
    for(int i = 0; i < _cutted_net.size(); ++i){
        int currentNetId = _cutted_net[i];
        int via_x = 0;
        int via_y = 0;
        via_x = (_cuttedNetIdToCM[currentNetId][0] - (double)_spacingTerminal / 2.0)/ (_sizeTerminalX + _spacingTerminal);
        via_x = max(via_x, 0);
        via_x = min(via_x, _num_via_x-1);
        via_y = (_cuttedNetIdToCM[currentNetId][1] - (double)_spacingTerminal / 2.0)/ (_sizeTerminalY + _spacingTerminal);
        via_y = max(via_y, 0);
        via_y = min(via_y, _num_via_y-1);
        _cuttedNetIdToNearestVia[currentNetId] = {via_x, via_y};
    }

    // via assignment

    int occ = 0;
    for(int i = 0; i < _cutted_net.size(); ++i){
        int currentNetId = _cutted_net[i];
        int index_x = _cuttedNetIdToNearestVia[currentNetId][0];
        int index_y = _cuttedNetIdToNearestVia[currentNetId][1];
        //cout << "via : " << _cuttedNetIdToNearestVia[currentNetId][0] << " , " << _cuttedNetIdToNearestVia[currentNetId][1] << endl;
        if(_occupiedViaMap[index_x][index_y]){
            //cout << "Occupied!! pos : " << index_x << " , " << index_y << endl;
            // BFS
            BFS_via(currentNetId,index_x, index_y);
            ++occ;
            //cout << "Find another via : " << _cuttedNetIdToNearestVia[currentNetId][0] << " , " << _cuttedNetIdToNearestVia[currentNetId][1] << endl;
        }
        else{
            _occupiedViaMap[index_x][index_y] = true;
        }
    }
    cout << "collision : " << occ << endl;
    cout << "num terminal : " << _cutted_net.size() << endl << endl;
    
}

void Placer::outputViaDecisionResult(string& fileName){

    // chack .gp.pl readed
    bool die = !(_partitioner->getPartSize(0) <= _partitioner->getPartSize(1));
    string postfix = (_partitioner->getPartSize(0) <= _partitioner->getPartSize(1))? "_top.gp.pl" : "_bot.gp.pl";
    fstream plFile;
    string plFileName = fileName + postfix;
    plFile.open(plFileName, ios::in);
    if(!plFile){
        return;
    }
    plFile.close();

    fstream fout;
    fstream fout2;

    // top/bot pl w/ terminals
    fout.open(fileName + "_top_term.pl", ios::out);
    fout2.open(fileName + "_bot_term.pl", ios::out);
    fout << "UCLA pl 1.0" << endl << endl << endl;
    fout2 << "UCLA pl 1.0" << endl << endl << endl;
    int num_term = _cutted_net.size();
    // write terminals
    for(int i = 0; i < num_term; ++i){
        int cuttedNetId = _cutted_net[i];
        int index_x = _cuttedNetIdToNearestVia[cuttedNetId][0];
        int index_y = _cuttedNetIdToNearestVia[cuttedNetId][1];
        int pos_x = _spacingTerminal + index_x * (_sizeTerminalX + _spacingTerminal);
        int pos_y = _spacingTerminal + index_y * (_sizeTerminalY + _spacingTerminal);;
        fout << "n" + to_string(_cutted_net[i]) << "\t" << pos_x << "\t" << pos_y << " : N" << endl;
        fout2 << "n" + to_string(_cutted_net[i]) << "\t" << pos_x << "\t" << pos_y << " : N" << endl;
    }
    // write modules
    for(int i = 0; i < _num_module; ++i){
        if(_partitioner->_cellArray[i]->getPart() == 0){
            fout << _modules[i].m_name << "\t" << _modules[i].m_x << "\t" << _modules[i].m_y << "\t" << 0 << " : N" << endl;
        }
        else{
            fout2 << _modules[i].m_name << "\t" << _modules[i].m_x << "\t" << _modules[i].m_y << "\t" << 0 << " : N" << endl;
        }
    }

    fout.close();
    fout2.close();

    // top/bot nodes w/ termernals
    fout.open(fileName + "_top_term.nodes", ios::out);
    fout2.open(fileName + "_bot_term.nodes", ios::out);
    fout << "UCLA nodes 1.0" << endl << endl << endl;
    fout2 << "UCLA nodes 1.0" << endl << endl << endl;
    fout << "NumNodes :\t" << _partitioner->getPartSize(0) + num_term << endl;
    fout2 << "NumNodes :\t" << _partitioner->getPartSize(1) + num_term << endl;
    fout << "NumTerminals :\t" << num_term << endl << endl;
    fout2 << "NumTerminals :\t" << num_term << endl << endl;
    for(int i = 0; i < num_term; ++i){
        fout << "n" + to_string(_cutted_net[i]) << "\t" << _sizeTerminalX << "\t" << _sizeTerminalY << "\t" << "terminal" << endl;
        fout2 << "n" + to_string(_cutted_net[i]) << "\t" << _sizeTerminalX << "\t" << _sizeTerminalY << "\t" << "terminal" << endl;
    }
    for(int i = 0; i < _num_module; ++i){
        if(_partitioner->_cellArray[i]->getPart() == 0){
            fout << _modules[i].m_name << "\t" << _techs[_topTech]._moduleTypes[_moduleIdToType[i]]._size_x << "\t";
            fout << _techs[_topTech]._moduleTypes[_moduleIdToType[i]]._size_y << endl;
        }
        else{
            fout2 << _modules[i].m_name << "\t" << _techs[_botTech]._moduleTypes[_moduleIdToType[i]]._size_x << "\t";
            fout2 << _techs[_botTech]._moduleTypes[_moduleIdToType[i]]._size_y << endl;
        }
    }

    fout.close();
    fout2.close();

    // top/bot nets w/ termernals
    fout.open(fileName + "_top_term.nets", ios::out);
    fout2.open(fileName + "_bot_term.nets", ios::out);
    fout << "UCLA nets 1.0" << endl << endl << endl;
    fout2 << "UCLA nets 1.0" << endl << endl << endl;
    int num_top_pin = 0;
    int num_bot_pin = 0;
    int num_top_net = 0;
    int num_bot_net = 0;
    string topNetInfo;
    string botNetInfo;
    for(int i = 0; i < _partitioner->getNetNum(); ++i){
        Net_p* currentNet = _partitioner->_netArray[i];
        vector<int> currentNetCellList = currentNet->getCellList();
        if(currentNet->getPartCount(0) != 0){
            ++num_top_net;
        }
        if(currentNet->getPartCount(1) != 0){
            ++num_bot_net;
        }
        int top_net_degree = 0;
        int bot_net_degree = 0;
        for(int j = 0; j < currentNetCellList.size(); ++j){
            int currentCellId = currentNetCellList[j];
            Cell* currentCell = _partitioner->_cellArray[currentNetCellList[j]];
            if(currentCell->getPart() == 0){
                ++top_net_degree;
            }
            if(currentCell->getPart() == 1){
                ++bot_net_degree;
            }
        }
        if(_netIdToIsCutted[i]){
            ++top_net_degree;
            ++bot_net_degree;
        }
        num_top_pin += top_net_degree;
        num_bot_pin += bot_net_degree;
    }
    
    fout << "NumNets :\t" << num_top_net << endl;
    fout2 << "NumNets :\t" << num_bot_net << endl;
    fout << "NumPins :\t" << num_top_pin << endl << endl;;
    fout2 << "NumPins :\t" << num_bot_pin << endl << endl;;
    // write nets
    for(int i = 0; i < _partitioner->getNetNum(); ++i){
        topNetInfo.clear();
        botNetInfo.clear();
        Net_p* currentNet = _partitioner->_netArray[i];
        vector<int> currentNetCellList = currentNet->getCellList();
        int top_net_degree = 0;
        int bot_net_degree = 0;
        // add terminals
        if(_netIdToIsCutted[i]){
            ++top_net_degree;
            ++bot_net_degree;
            topNetInfo.append("\t");
            topNetInfo.append("n" + to_string(i));
            topNetInfo.append("\tI\n");
            botNetInfo.append("\t");
            botNetInfo.append("n" + to_string(i));
            botNetInfo.append("\tI\n");
        }
        for(int j = 0; j < currentNetCellList.size(); ++j){
            int currentCellId = currentNetCellList[j];
            Cell* currentCell = _partitioner->_cellArray[currentNetCellList[j]];
            if(currentCell->getPart() == 0){
                ++top_net_degree;
                topNetInfo.append("\t");
                topNetInfo.append(_modules[currentCellId].GetName());
                topNetInfo.append("\tI : ");
                double offset_x = (double)_techs[_topTech]._moduleTypes[_moduleIdToType[currentCellId]]._size_x / 2.0;
                double offset_y = (double)_techs[_topTech]._moduleTypes[_moduleIdToType[currentCellId]]._size_y / 2.0;
                topNetInfo.append(to_string(_techs[_topTech]._moduleTypes[_moduleIdToType[currentCellId]]._pins[_nets[i][j][1]][0] - offset_x));
                topNetInfo.append(" ");
                topNetInfo.append(to_string(_techs[_topTech]._moduleTypes[_moduleIdToType[currentCellId]]._pins[_nets[i][j][1]][1] - offset_y));
                topNetInfo.append("\n");
            }
            if(currentCell->getPart() == 1){
                ++bot_net_degree;
                botNetInfo.append("\t");
                botNetInfo.append(_modules[currentCellId].GetName());
                botNetInfo.append("\tI : ");
                double offset_x = (double)_techs[_botTech]._moduleTypes[_moduleIdToType[currentCellId]]._size_x / 2.0;
                double offset_y = (double)_techs[_botTech]._moduleTypes[_moduleIdToType[currentCellId]]._size_y / 2.0;
                botNetInfo.append(to_string(_techs[_botTech]._moduleTypes[_moduleIdToType[currentCellId]]._pins[_nets[i][j][1]][0] - offset_x));
                botNetInfo.append(" ");
                botNetInfo.append(to_string(_techs[_botTech]._moduleTypes[_moduleIdToType[currentCellId]]._pins[_nets[i][j][1]][1] - offset_y));
                botNetInfo.append("\n");
            }
        }
        if(top_net_degree){
            fout << "NetDegree :\t" << top_net_degree << endl;
            fout << topNetInfo;
        }
        if(bot_net_degree){
            fout2 << "NetDegree :\t" << bot_net_degree << endl;
            fout2 << botNetInfo;
        }
    }

    fout.close();
    fout2.close();

    // top/bot aux w/ terminals
    size_t pos = fileName.rfind("/");
    string caseName;
    if(pos != string::npos){
        caseName = fileName.substr(pos+1);
    }
    else{
        caseName = fileName;
    }
    fout.open(fileName + "_top_term.aux", ios::out);
    fout2.open(fileName + "_bot_term.aux", ios::out);
    fout << "RowBasedPlacement : ";
    fout2 << "RowBasedPlacement : ";
    string topAux;
    string botAux;
    topAux.append(caseName + "_top_term.nodes " + caseName + "_top_term.nets " + caseName + "_top.wts " + caseName + "_top_term.pl " + caseName + "_top.scl");
    botAux.append(caseName + "_bot_term.nodes " + caseName + "_bot_term.nets " + caseName + "_bot.wts " + caseName + "_bot_term.pl " + caseName + "_bot.scl");
    fout << topAux;
    fout2 << botAux;
    
    fout.close();
    fout2.close();

    cout << "terminal output done." << endl << endl;

}


