#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <map>
#include "partitioner.h"

using namespace std;

void Partitioner::initialize(vector<int>& m_top_list, vector<int>& m_bot_list, vector<vector<int>>& n_list)
{
    _cellNum = m_top_list.size() + m_bot_list.size();
    _netNum = n_list.size();

    for(int i = 0; i < _cellNum; ++i){
        _cellArray.push_back(new Cell(0, i));
    }

    for(int i = 0; i < n_list.size(); ++i){

        _netArray.push_back(new Net_p);

        for(int j = 0; j < n_list[i].size(); ++j){
            _cellArray[n_list[i][j]]->addNet(i);
            _cellArray[n_list[i][j]]->incPinNum();
            _netArray[i]->addCell(n_list[i][j]);
        }

    }
    
    // find Pmax
    _maxPinNum = (*_cellArray.begin())->getPinNum();
    for(vector<Cell*>::iterator it = ++(_cellArray.begin()); it != _cellArray.end(); ++it){
        if(_maxPinNum < (*it)->getPinNum()){
            _maxPinNum = (*it)->getPinNum();
        }
    }

    // initialize buckets
    for(int i = -_maxPinNum; i <= _maxPinNum; ++i){
        // id == -1 means it is a head node. Head node contains nothing.
        _bList[0][i] = new Node(-1);
        _bList[1][i] = new Node(-1);
    }

    // devide cells into 2 parts
    for(int i = 0; i < m_top_list.size(); ++i){
        int currentId = m_top_list[i];
        _cellArray[currentId]->setPart(0);
        ++_partSize[0];
        vector<int> tmpNetList = _cellArray[currentId]->getNetList();
        for(vector<int>::iterator it = tmpNetList.begin(); it != tmpNetList.end(); ++it){
            //cout << currentId << " : net " << (*it) << " p: " << _netArray[(*it)]->getPartCount(0) << endl;
            _netArray[(*it)]->incPartCount(0);
        }
        _cellArray[currentId]->setGain(0);
        _topUtilArea += _techs[_topTech]._moduleTypes[_cellIdToType[currentId]]._area;
    }

    for(int i = 0; i < m_bot_list.size(); ++i){
        int currentId  = m_bot_list[i];
        _cellArray[currentId]->setPart(1);
        ++_partSize[1];
        vector<int> tmpNetList = _cellArray[currentId]->getNetList();
        for(vector<int>::iterator it = tmpNetList.begin(); it != tmpNetList.end(); ++it){
            //cout << currentId << " : net " << (*it) << " p: " << _netArray[(*it)]->getPartCount(1) << endl;
            _netArray[(*it)]->incPartCount(1);
        }
        _cellArray[currentId]->setGain(0);
        _botUtilArea += _techs[_botTech]._moduleTypes[_cellIdToType[currentId]]._area;
    }

    _topCurrentUtilArea = _topUtilArea;
    _botCurrentUtilArea = _botUtilArea;

}

void Partitioner::computeGain()
{
    // calculate initial cut size and gains of cells , area
    _topUtilArea = 0;
    _botUtilArea = 0;
    for(int i = 0; i < _cellNum; ++i){
        _cellArray[i]->setGain(0);
        if(_cellArray[i]->getPart() == 0){
            _topUtilArea += _techs[_topTech]._moduleTypes[_cellIdToType[i]]._area;
        }
        if(_cellArray[i]->getPart() == 1){
            _botUtilArea += _techs[_botTech]._moduleTypes[_cellIdToType[i]]._area;
        }
    }
    _cutSize = 0;
    for(vector<Net_p*>::iterator it = _netArray.begin(); it != _netArray.end(); ++it){
        // cut size
        if((*it)->getPartCount(0) && (*it)->getPartCount(1)){
            ++_cutSize;
            //cout << "*" << _cutSize << endl;
        }
        // gain
        vector<int> tmpCellList = (*it)->getCellList();
        for(vector<int>::iterator jt = tmpCellList.begin(); jt != tmpCellList.end(); ++jt){
            // F
            if(((*it)->getPartCount(_cellArray[(*jt)]->getPart())) == 1){
                _cellArray[(*jt)]->incGain();
            }
            // T
            if(((*it)->getPartCount(!(_cellArray[(*jt)]->getPart()))) == 0){
                _cellArray[(*jt)]->decGain();
            }
        }
    }

    // put cells into buckets
    int tmpMaxGainA = -_maxPinNum;
    int tmpMaxGainB = -_maxPinNum;
    _maxGainCellA = _bList[0][tmpMaxGainA];
    _maxGainCellB = _bList[1][tmpMaxGainB];
    for(int i = 0; i < _cellNum; ++i){
        Node* nodePtr = new Node(i);
        _cellArray[i]->setNode(nodePtr);
        //cout << "gain " << _cellArray[i]->getGain() <<endl;
        if(!(_bList[_cellArray[i]->getPart()][_cellArray[i]->getGain()]->getNext())){
            nodePtr->setPrev(_bList[_cellArray[i]->getPart()][_cellArray[i]->getGain()]);
            _bList[_cellArray[i]->getPart()][_cellArray[i]->getGain()]->setNext(nodePtr);
        }
        else{
            // insert
            nodePtr->setPrev(_bList[_cellArray[i]->getPart()][_cellArray[i]->getGain()]);
            nodePtr->setNext(_bList[_cellArray[i]->getPart()][_cellArray[i]->getGain()]->getNext());
            (_bList[_cellArray[i]->getPart()][_cellArray[i]->getGain()]->getNext())->setPrev(nodePtr);
            _bList[_cellArray[i]->getPart()][_cellArray[i]->getGain()]->setNext(nodePtr);
        }
        // update _maxGainCell
        if(_cellArray[i]->getPart() == 0){
            if(tmpMaxGainA < _cellArray[i]->getGain()){
                _maxGainCellA = _bList[0][_cellArray[i]->getGain()];
                tmpMaxGainA = _cellArray[i]->getGain();
            }
        }
        else{
            if(tmpMaxGainB < _cellArray[i]->getGain()){
                _maxGainCellB = _bList[1][_cellArray[i]->getGain()];
                tmpMaxGainB = _cellArray[i]->getGain();
            }
        }
    }
}

int Partitioner::moveMaxGainCell(){
    int movedCellId = -1;
    if(_maxGainCellA->getNext() && _maxGainCellB->getNext()){
        // A -> B
        if((_cellArray[_maxGainCellA->getNext()->getId()]->getGain() >= _cellArray[_maxGainCellB->getNext()->getId()]->getGain())
        && ((_botCurrentUtilArea + _techs[_botTech]._moduleTypes[_cellIdToType[_maxGainCellA->getNext()->getId()]]._area)/_dieArea < _botMaxUtil)){
            //cout << (_botCurrentUtilArea + _techs[_botTech]._moduleTypes[_cellIdToType[_maxGainCellA->getNext()->getId()]]._area)/_dieArea  << endl; 
            // get id
            movedCellId = _maxGainCellA->getNext()->getId();
            // push cell into move stack
            _moveStack.push_back(movedCellId);
            // adjust part sizes
            --_partSize[0];
            ++_partSize[1];
            // remove cell from the bucket
            Node* tmpNodePtr = _maxGainCellA->getNext();
            if(tmpNodePtr->getNext()){
                tmpNodePtr->getNext()->setPrev(_maxGainCellA);
            }
            _maxGainCellA->setNext(tmpNodePtr->getNext());
            delete tmpNodePtr;
        }
        // B -> A
        else if(((_topCurrentUtilArea + _techs[_topTech]._moduleTypes[_cellIdToType[_maxGainCellB->getNext()->getId()]]._area)/_dieArea < _topMaxUtil)){
            // get id
            movedCellId = _maxGainCellB->getNext()->getId();
            // push cell into move stack
            _moveStack.push_back(movedCellId);
            // adjust part sizes
            --_partSize[1];
            ++_partSize[0];
            // remove cell from the bucket
            Node* tmpNodePtr = _maxGainCellB->getNext();
            if(tmpNodePtr->getNext()){
                tmpNodePtr->getNext()->setPrev(_maxGainCellB);
            }
            _maxGainCellB->setNext(tmpNodePtr->getNext());
            delete tmpNodePtr;
        }
        // A -> B
        else if(((_botCurrentUtilArea + _techs[_botTech]._moduleTypes[_cellIdToType[_maxGainCellA->getNext()->getId()]]._area)/_dieArea < _botMaxUtil)){
            // get id
            movedCellId = _maxGainCellA->getNext()->getId();
            // push cell into move stack
            _moveStack.push_back(movedCellId);
            // adjust part sizes
            --_partSize[0];
            ++_partSize[1];
            // remove cell from the bucket
            Node* tmpNodePtr = _maxGainCellA->getNext();
            if(tmpNodePtr->getNext()){
                tmpNodePtr->getNext()->setPrev(_maxGainCellA);
            }
            _maxGainCellA->setNext(tmpNodePtr->getNext());
            delete tmpNodePtr;
        }
    }
    else if(_maxGainCellA->getNext()){
        // A ->B
        if(((_botCurrentUtilArea + _techs[_botTech]._moduleTypes[_cellIdToType[_maxGainCellA->getNext()->getId()]]._area)/_dieArea < _botMaxUtil)){
            // get id
            movedCellId = _maxGainCellA->getNext()->getId();
            // push cell into move stack
            _moveStack.push_back(movedCellId);
            // adjust part sizes
            --_partSize[0];
            ++_partSize[1];
            // remove cell from the bucket
            Node* tmpNodePtr = _maxGainCellA->getNext();
            if(tmpNodePtr->getNext()){
                tmpNodePtr->getNext()->setPrev(_maxGainCellA);
            }
            _maxGainCellA->setNext(tmpNodePtr->getNext());
            delete tmpNodePtr;
        }
    }
    else if(_maxGainCellB->getNext()){
        // B -> A
        if(((_topCurrentUtilArea + _techs[_topTech]._moduleTypes[_cellIdToType[_maxGainCellB->getNext()->getId()]]._area)/_dieArea < _topMaxUtil)){
            // get id
            movedCellId = _maxGainCellB->getNext()->getId();
            // push cell into move stack
            _moveStack.push_back(movedCellId);
            // adjust part sizes
            --_partSize[1];
            ++_partSize[0];
            // remove cell from the bucket
            Node* tmpNodePtr = _maxGainCellB->getNext();
            if(tmpNodePtr->getNext()){
                tmpNodePtr->getNext()->setPrev(_maxGainCellB);
            }
            _maxGainCellB->setNext(tmpNodePtr->getNext());
            delete tmpNodePtr;
        }
    }

    return movedCellId;
}

void Partitioner::updateGain(int id)
{
    // the cell has not moved
    bool F = _cellArray[id]->getPart();
    bool T = !(_cellArray[id]->getPart());
    // lock the base cell
    _cellArray[id]->lock();
    vector<int> tmpNetList = _cellArray[id]->getNetList();
    for(vector<int>::iterator it = tmpNetList.begin(); it != tmpNetList.end(); ++it){
        vector<int> tmpCellList = _netArray[*it]->getCellList();
        if(_netArray[*it]->getPartCount(T) == 0){
            for(vector<int>::iterator jt = tmpCellList.begin(); jt != tmpCellList.end(); ++jt){
                if(!(_cellArray[*jt]->getLock())){
                    _cellArray[*jt]->incGain();
                    int tmpGain = _cellArray[*jt]->getGain();
                    Node* tmpNodePtr = _cellArray[*jt]->getNode();
                    if(tmpNodePtr->getNext()){
                        tmpNodePtr->getNext()->setPrev(tmpNodePtr->getPrev());
                    }
                    tmpNodePtr->getPrev()->setNext(tmpNodePtr->getNext());
                    tmpNodePtr->setPrev(_bList[F][tmpGain]);
                    tmpNodePtr->setNext(_bList[F][tmpGain]->getNext());
                    _bList[F][tmpGain]->setNext(tmpNodePtr);
                    if(tmpNodePtr->getNext()){
                        tmpNodePtr->getNext()->setPrev(tmpNodePtr);
                    }
                }
            }
        }
        else if(_netArray[*it]->getPartCount(T) == 1){
            for(vector<int>::iterator jt = tmpCellList.begin(); jt != tmpCellList.end(); ++jt){
                if(!(_cellArray[*jt]->getLock()) && (_cellArray[*jt]->getPart() == T)){
                    _cellArray[*jt]->decGain();
                    int tmpGain = _cellArray[*jt]->getGain();
                    Node* tmpNodePtr = _cellArray[*jt]->getNode();
                    if(tmpNodePtr->getNext()){
                        tmpNodePtr->getNext()->setPrev(tmpNodePtr->getPrev());
                    }
                    tmpNodePtr->getPrev()->setNext(tmpNodePtr->getNext());
                    tmpNodePtr->setPrev(_bList[T][tmpGain]);
                    tmpNodePtr->setNext(_bList[T][tmpGain]->getNext());
                    _bList[T][tmpGain]->setNext(tmpNodePtr);
                    if(tmpNodePtr->getNext()){
                        tmpNodePtr->getNext()->setPrev(tmpNodePtr);
                    }
                }
            }
        }
        _netArray[*it]->decPartCount(F);
        _netArray[*it]->incPartCount(T);
        if(_netArray[*it]->getPartCount(F) == 0){
            for(vector<int>::iterator jt = tmpCellList.begin(); jt != tmpCellList.end(); ++jt){
                if(!(_cellArray[*jt]->getLock())){
                    _cellArray[*jt]->decGain();
                    int tmpGain = _cellArray[*jt]->getGain();
                    Node* tmpNodePtr = _cellArray[*jt]->getNode();
                    if(tmpNodePtr->getNext()){
                        tmpNodePtr->getNext()->setPrev(tmpNodePtr->getPrev());
                    }
                    tmpNodePtr->getPrev()->setNext(tmpNodePtr->getNext());
                    tmpNodePtr->setPrev(_bList[T][tmpGain]);
                    tmpNodePtr->setNext(_bList[T][tmpGain]->getNext());
                    _bList[T][tmpGain]->setNext(tmpNodePtr);
                    if(tmpNodePtr->getNext()){
                        tmpNodePtr->getNext()->setPrev(tmpNodePtr);
                    }
                }
            }
        }
        else if(_netArray[*it]->getPartCount(F) == 1){
            for(vector<int>::iterator jt = tmpCellList.begin(); jt != tmpCellList.end(); ++jt){
                if(!(_cellArray[*jt]->getLock()) && (_cellArray[*jt]->getPart() == F)){
                    _cellArray[*jt]->incGain();
                    int tmpGain = _cellArray[*jt]->getGain();
                    Node* tmpNodePtr = _cellArray[*jt]->getNode();
                    if(tmpNodePtr->getNext()){
                        tmpNodePtr->getNext()->setPrev(tmpNodePtr->getPrev());
                    }
                    tmpNodePtr->getPrev()->setNext(tmpNodePtr->getNext());
                    tmpNodePtr->setPrev(_bList[F][tmpGain]);
                    tmpNodePtr->setNext(_bList[F][tmpGain]->getNext());
                    _bList[F][tmpGain]->setNext(tmpNodePtr);
                    if(tmpNodePtr->getNext()){
                        tmpNodePtr->getNext()->setPrev(tmpNodePtr);
                    }
                }
            }
        }
    }
    // update max gain
    _maxGainCellA = _bList[0][-_maxPinNum];
    for(int i = _maxPinNum; i > -_maxPinNum; --i){
        if(_bList[0][i]->getNext()){
            _maxGainCellA = _bList[0][i];
            break;
        }
    }
    _maxGainCellB = _bList[1][-_maxPinNum];
    for(int i = _maxPinNum; i > -_maxPinNum; --i){
        if(_bList[1][i]->getNext()){
            _maxGainCellB = _bList[1][i];
            break;
        }
    }
    // update area
    if(F == 0){
        _topCurrentUtilArea -= _techs[_topTech]._moduleTypes[_cellIdToType[id]]._area;
        _botCurrentUtilArea += _techs[_botTech]._moduleTypes[_cellIdToType[id]]._area;
    }
    if(F == 1){
        _topCurrentUtilArea += _techs[_topTech]._moduleTypes[_cellIdToType[id]]._area;
        _botCurrentUtilArea -= _techs[_botTech]._moduleTypes[_cellIdToType[id]]._area;
    }


}

void Partitioner::partition()
{

    // start loop
    _iterNum = 1;
    do{
        computeGain();
        _accGain = 0;
        _maxAccGain = 0;
        _bestMoveNum = 0;
        int originalPartSizeA = _partSize[0];
        int originalPartSizeB = _partSize[1];
        _topCurrentUtilArea = _topUtilArea;
        _botCurrentUtilArea = _botUtilArea;
        cout << "cutSize: " <<_cutSize << endl;
        for(_moveNum = 1; _moveNum <= _cellNum; ++_moveNum){
            int moveCellId = moveMaxGainCell();
            // check if a cell has been moved
            if(moveCellId == -1){
                break;
            }
            else{
                _accGain += _cellArray[moveCellId]->getGain();
                //cout << "cell: " << moveCellId << "  g: " << _cellArray[moveCellId]->getGain() << "  acc: " << _accGain << endl;
                if(_moveNum == 1){
                    _maxAccGain = _accGain;
                    _bestMoveNum = _moveNum;
                }
                if(_maxAccGain < _accGain){
                    _maxAccGain = _accGain;
                    _bestMoveNum = _moveNum;
                }
                updateGain(moveCellId);
            }
        }
        cout << "iteration: " << _iterNum << " , Max gain: " << _maxAccGain << " , A size: " << _partSize[0] << " , B size: " << _partSize[1];
        cout << " , best move num: " << _bestMoveNum << endl;

        if(_maxAccGain > 0){
            // move cells and calculate part sizes
            _partSize[0] = originalPartSizeA;
            _partSize[1] = originalPartSizeB;
            for(int i = 0; i < _bestMoveNum; ++i){
                //cout << _moveStack[i] << endl;
                _cellArray[_moveStack[i]]->move();
                ++_partSize[_cellArray[_moveStack[i]]->getPart()];
                --_partSize[!(_cellArray[_moveStack[i]]->getPart())];
                if(_cellArray[_moveStack[i]]->getPart() == 0){
                    _topUtilArea += _techs[_topTech]._moduleTypes[_cellIdToType[_moveStack[i]]]._area;
                    _botUtilArea -= _techs[_botTech]._moduleTypes[_cellIdToType[_moveStack[i]]]._area;
                }
                if(_cellArray[_moveStack[i]]->getPart() == 1){
                    _topUtilArea -= _techs[_topTech]._moduleTypes[_cellIdToType[_moveStack[i]]]._area;
                    _botUtilArea += _techs[_botTech]._moduleTypes[_cellIdToType[_moveStack[i]]]._area;
                }
            }
            // calculate part counts of nets and unlock cells
            for(vector<Net_p*>::iterator it = _netArray.begin(); it != _netArray.end(); ++it){
                (*it)->setPartCount(0, 0);
                (*it)->setPartCount(1, 0);
            }
            for(int i = 0; i < _cellNum; ++i){
                vector<int> tmpNetList = _cellArray[i]->getNetList();
                for(vector<int>::iterator it = tmpNetList.begin(); it != tmpNetList.end(); ++it){
                    _netArray[(*it)]->incPartCount(_cellArray[i]->getPart());
                }
                // unlock cells
                _cellArray[i]->unlock();
            }
            // calculate the cut size
            _cutSize -= _maxAccGain;
        }
        else{
            _partSize[0] = originalPartSizeA;
            _partSize[1] = originalPartSizeB;
            // calculate part counts of nets
            for(vector<Net_p*>::iterator it = _netArray.begin(); it != _netArray.end(); ++it){
                (*it)->setPartCount(0, 0);
                (*it)->setPartCount(1, 0);
            }
            for(int i = 0; i < _cellNum; ++i){
                vector<int> tmpNetList = _cellArray[i]->getNetList();
                for(vector<int>::iterator it = tmpNetList.begin(); it != tmpNetList.end(); ++it){
                    _netArray[(*it)]->incPartCount(_cellArray[i]->getPart());
                }
                
            }
        }

        
        cout << "topMaxUtil: " << _topMaxUtil << " , topUtil: " << _topUtilArea/_dieArea << endl;
        cout << "botMaxUtil: " << _botMaxUtil << " , botUtil: " << _botUtilArea/_dieArea << endl << endl;

        // erase the move stack
        _moveStack.clear();

        ++_iterNum;

    }while(_maxAccGain > 0);
    
    // verify the answer
    cout << endl << "verify:" << endl;
    int sizeA = 0;
    for(int i = 0; i < _cellNum; ++i){
        if(_cellArray[i]->getPart() == 0){
            ++sizeA;
        }
    }
    cout << "size A: " << _partSize[0] << " , " << sizeA << endl; 
    int sizeB = 0;
    for(int i = 0; i < _cellNum; ++i){
        if(_cellArray[i]->getPart() == 1){
            ++sizeB;
        }
    }
    cout << "size B: " << _partSize[1] << " , " << sizeB << endl; 
    int testCutSize = 0;
    for(vector<Net_p*>::iterator it = _netArray.begin(); it != _netArray.end(); ++it){
        vector<int> cellList = (*it)->getCellList();
        bool inA = 0;
        bool inB = 0;
        for(vector<int>::iterator jt = cellList.begin(); jt != cellList.end(); ++jt){
            if(_cellArray[*jt]->getPart() == 0) inA = 1;
            if(_cellArray[*jt]->getPart() == 1) inB = 1;
        }
        if(inA && inB) ++testCutSize;
    }
    cout << "cut size: " << _cutSize << " , " << testCutSize << endl;

}

void Partitioner::printSummary() const
{
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << _cutSize << endl;
    cout << " Total cell number: " << _cellNum << endl;
    cout << " Total net number:  " << _netNum << endl;
    cout << " Cell Number of partition A: " << _partSize[0] << endl;
    cout << " Cell Number of partition B: " << _partSize[1] << endl;
    cout << "=================================================" << endl;
    cout << "topMaxUtil: " << _topMaxUtil << " , topUtil: " << _topUtilArea/_dieArea << endl;
    cout << "botMaxUtil: " << _botMaxUtil << " , botUtil: " << _botUtilArea/_dieArea << endl;
    cout << endl;
    return;
}
/*
void Partitioner::reportNet() const
{
    cout << "Number of nets: " << _netNum << endl;
    for (size_t i = 0, end_i = _netArray.size(); i < end_i; ++i) {
        cout << setw(8) << _netArray[i]->getName() << ": ";
        vector<int> cellList = _netArray[i]->getCellList();
        for (size_t j = 0, end_j = cellList.size(); j < end_j; ++j) {
            cout << setw(8) << _cellArray[cellList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::reportCell() const
{
    cout << "Number of cells: " << _cellNum << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
        cout << setw(8) << _cellArray[i]->getName() << ": ";
        vector<int> netList = _cellArray[i]->getNetList();
        for (size_t j = 0, end_j = netList.size(); j < end_j; ++j) {
            cout << setw(8) << _netArray[netList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::writeResult(fstream& outFile)
{
    stringstream buff;
    buff << _cutSize;
    outFile << "Cutsize = " << buff.str() << '\n';
    buff.str("");
    buff << _partSize[0];
    outFile << "G1 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 0) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    buff.str("");
    buff << _partSize[1];
    outFile << "G2 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 1) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    return;
}
*/

void Partitioner::clear()
{
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        delete _cellArray[i];
    }
    for (size_t i = 0, end = _netArray.size(); i < end; ++i) {
        delete _netArray[i];
    }
    return;
}
