#ifndef PLACEDB_H
#define PLACEDB_H
/**
 *
@author Indark#eda
Placement Database
2005-10-25
*/

// 2007-07-17 (donnie)
// I removed the liberty information connected to CPlaceDB to save memory.
// The liberty info should not be embedded into Pins directly to save memory.


#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <cassert>
#include <set>
#include <fstream>
#include "partitioner.h"
#include "tech.h"

using namespace std;

enum ORIENT{
	OR_N,OR_W,OR_S,OR_E,OR_FN,OR_FW,OR_FS,OR_FE
//         0    1    2    3    4     5     6     7	    
//         |    -    |    -    |     -     |     -
	};


enum BLOCK_TYPE { BT_OTHER, BT_PI, BT_PO, BT_P_INOUT, BT_PIN, BT_DFF };

class Module
{
    public:
	
	Module()
	{
	    Init();
	}

	Module( string name, bool die = 0, short techId = 0, short type = 0, float width = 0, float height = 0, bool isFixed=false ) 
	{
	    Init();
	    m_x = (float)0.0;
	    m_y = (float)0.0;
	    m_name    = name;
        m_die     = die;
        m_tech    = techId;
        m_type    = type;
	    m_width   = width;
	    m_height  = height;
	    m_area    = width*height;
	    m_isFixed = isFixed;
	    assert( m_area >= 0 );
	}
	
	void Init()
	{
	    m_width  = -1;
	    m_height = -1;
	    m_area   = -1;
	    m_isDummy   = false;
	    m_isCluster = false;
	    m_blockType = BT_OTHER;
	    m_isFixed   = false;
	}
	
	void CalcCenter()
	{ 
	    m_cx = m_x + (float)0.5 * m_width;
	    m_cy = m_y + (float)0.5 * m_height;
	}

	string GetName()		{ return m_name; } 
	float GetWidth()		{ return m_width; }
	float GetHeight()		{ return m_height; }
	float GetX()			{ return m_x; }
	float GetY()			{ return m_y; }
	float GetArea()			{ return m_area; }

	float  m_x, m_y;
	float  m_cx, m_cy;
	string m_name;
	float  m_width, m_height;
	float  m_area;
    short int m_tech;
    bool m_die;             // 0 : top, 1 : bot
	short int m_type;	    // module type id (in cell library)
	bool   m_isFixed;	    // fixed block or pads
	//bool   m_isPin;             // 2006-05-22 determine PIs and POs 
	//bool   m_isOutCore;
	bool   m_isDummy;	    // 2006-03-02 (donnie)
	bool   m_isCluster;         // 2006-03-20 (tchsu)
	bool   m_isMacro;	    // 2007-01-05 (indark)
	BLOCK_TYPE m_blockType;	    // 2006-06-22 (donnie)

	vector<int> m_pinsId;
	vector<int> m_netsId;

	vector<int> m_inBlocksId;	// for TimeMachine
	vector<int> m_outBlocksId;	// for TimeMachine

	bool isRotated();
};


class Placer
{
public:

    Placer(fstream& inFile){ 
        parseInput(inFile);
    }
    ~Placer(){ }


    void parseInput(fstream& inFile);
    void initialPartition();
    void outputPartitionResult(string&);
	void BFS_via(int ,int, int);
	void viaDecision(string&);
	void outputViaDecisionResult(string&);

    int _num_techs;

    vector<Tech> _techs;

    map<string, int> _techNameToId;
    map<string, int> _typeNameToId;
    map<string, int> _pinNameToId;

    double _dieLLX;
    double _dieLLY;
    double _dieURX;
    double _dieURY;

    double _dieArea;

    double _topMaxUtil;
    double _botMaxUtil;

    int _topRowLocX;
    int _topRowLocY;
    int _topRowWidth;
    int _topRowHeight;
    int _num_topRow;

    int _botRowLocX;
    int _botRowLocY;
    int _botRowWidth;
    int _botRowHeight;
    int _num_botRow;

    int _topTech;
    int _botTech;

    int _sizeTerminalX;
    int _sizeTerminalY;
    int _spacingTerminal;

	bool _terminalDecisionDie;

    Partitioner* _partitioner;

    int _num_module;
    vector<Module> _modules;
    map<string, int> _moduleNameToId; 

    int _num_net;
    vector<vector<vector<int>>> _nets;
    map<string, int> _netNameToId;
	vector<string> _netIdToName;

    vector<int> _moduleIdToType;

	vector<int> _cutted_net; // calculated in viaDecision().
	vector<bool> _netIdToIsCutted;
	map<int, vector<double>> _cuttedNetIdToCM;
	map<int, double> _cuttedNetIdToHPWL;

	int _num_via_x;
	int _num_via_y;

	map<int, vector<int>> _cuttedNetIdToNearestVia; // "cutted" id

	vector<vector<bool>> _occupiedViaMap; // 1 : occupied, 0 : vacant


};



#endif