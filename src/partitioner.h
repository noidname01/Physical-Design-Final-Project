#ifndef PARTITIONER_H
#define PARTITIONER_H

#include <fstream>
#include <vector>
#include <map>
#include "tech.h"

using namespace std;

class Node
{
    friend class Cell;

public:
    // Constructor and destructor
    Node(const int& id) :
        _id(id), _prev(NULL), _next(NULL) { }
    ~Node() { }

    // Basic access methods
    int getId() const       { return _id; }
    Node* getPrev() const   { return _prev; }
    Node* getNext() const   { return _next; }

    // Set functions
    void setId(const int& id) { _id = id; }
    void setPrev(Node* prev)  { _prev = prev; }
    void setNext(Node* next)  { _next = next; }

private:
    int         _id;    // id of the node (indicating the cell)
    Node*       _prev;  // pointer to the previous node
    Node*       _next;  // pointer to the next node
};

class Cell
{
public:
    // Constructor and destructor
    Cell(bool part, int id) :
        _gain(0), _pinNum(0), _part(part), _lock(false) {
        //_node = new Node(id);
    }
    ~Cell() { }

    // Basic access methods
    int getGain() const     { return _gain; }
    int getPinNum() const   { return _pinNum; }
    bool getPart() const    { return _part; }
    bool getLock() const    { return _lock; }
    Node* getNode() const   { return _node; }
    int getFirstNet() const { return _netList[0]; }
    vector<int> getNetList() const  { return _netList; }

    // Set functions
    void setNode(Node* node)        { _node = node; }
    void setGain(const int gain)    { _gain = gain; }
    void setPart(const bool part)   { _part = part; }

    // Modify methods
    void move()         { _part = !_part; }
    void lock()         { _lock = true; }
    void unlock()       { _lock = false; }
    void incGain()      { ++_gain; }
    void decGain()      { --_gain; }
    void incPinNum()    { ++_pinNum; }
    void decPinNum()    { --_pinNum; }
    void addNet(const int netId) { _netList.push_back(netId); }

private:
    int             _gain;      // gain of the cell
    int             _pinNum;    // number of pins the cell are connected to
    bool            _part;      // partition the cell belongs to (0-A, 1-B)
    bool            _lock;      // whether the cell is locked
    Node*           _node;      // node used to link the cells together
    vector<int>     _netList;   // list of nets the cell is connected to
};


class Net_p
{
public:
    // constructor and destructor
    Net_p()
    {
        _partCount[0] = 0; _partCount[1] = 0;
    }
    ~Net_p()  { }

    // basic access methods
    int getPartCount(int part) const { return _partCount[part]; }
    vector<int> getCellList()  const { return _cellList; }

    // set functions
    void setPartCount(int part, const int count) { _partCount[part] = count; }

    // modify methods
    void incPartCount(int part)     { ++_partCount[part]; }
    void decPartCount(int part)     { --_partCount[part]; }
    void addCell(const int cellId)  { _cellList.push_back(cellId); }

private:
    int             _partCount[2];  // Cell number in partition A(0) and B(1)
    vector<int>     _cellList;      // List of cells the net is connected to
};


class Partitioner
{
    friend class Placer;
public:
    // constructor and destructor
    Partitioner(double dieArea, double topMaxUtil, double botMaxUtil, vector<int>& m_top_list, vector<int>& m_bot_list,
        vector<vector<int>>& n_list, vector<Tech>& techs, int topTech, int botTech, vector<int>& moduleIdToType) :
        _cutSize(0), _netNum(0), _cellNum(0), _maxPinNum(0), _dieArea(dieArea),
        _topMaxUtil(topMaxUtil/100.0), _botMaxUtil(botMaxUtil/100.0), _techs(techs),
        _topTech(topTech), _botTech(botTech), _topUtilArea(0), _botUtilArea(0), _cellIdToType(moduleIdToType),
        _accGain(0), _maxAccGain(0), _iterNum(0) {
        _partSize[0] = 0;
        _partSize[1] = 0;
        initialize(m_top_list, m_bot_list, n_list);
    }
    ~Partitioner() {
        clear();
    }

    // basic access methods
    int getCutSize() const          { return _cutSize; }
    int getNetNum() const           { return _netNum; }
    int getCellNum() const          { return _cellNum; }
    //double getBFactor() const       { return _bFactor; }
    int getPartSize(int part) const { return _partSize[part]; }

    // modify method
    void initialize(vector<int>&, vector<int>&, vector<vector<int>>&);
    void computeGain();
    int moveMaxGainCell();
    void updateGain(int id);
    void partition();

    // member functions about reporting
    void printSummary() const;
    void reportNet() const;
    void reportCell() const;
    void writeResult(fstream& outFile);

private:
    int                 _cutSize;       // cut size
    int                 _partSize[2];   // size (cell number) of partition A(0) and B(1)
    int                 _netNum;        // number of nets
    int                 _cellNum;       // number of cells
    int                 _maxPinNum;     // Pmax for building bucket list
    double              _topMaxUtil; 
    double              _botMaxUtil;  
    double              _dieArea;     
    double              _topUtilArea;
    double              _botUtilArea;
    double              _topCurrentUtilArea;
    double              _botCurrentUtilArea;
    int                 _topTech;
    int                 _botTech;
    Node*               _maxGainCellA;   // pointer to max gain cell in A
    Node*               _maxGainCellB;   // pointer to max gain cell in B
    vector<Net_p*>      _netArray;      // net array of the circuit
    vector<Cell*>       _cellArray;     // cell array of the circuit
    map<int, Node*>     _bList[2];      // bucket list of partition A(0) and B(1)
    vector<Tech>        _techs;
    vector<int>         _cellIdToType;

    int                 _accGain;       // accumulative gain
    int                 _maxAccGain;    // maximum accumulative gain
    int                 _moveNum;       // number of cell movements
    int                 _iterNum;       // number of iterations
    int                 _bestMoveNum;   // store best number of movements
    int                 _unlockNum[2];  // number of unlocked cells
    vector<int>         _moveStack;     // history of cell movement

    // Clean up partitioner
    void clear();
};

#endif  // PARTITIONER_H
