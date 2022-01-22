/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>

using namespace std;

#include "cirDef.h"
#include "../sat/sat.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr():flAIG(0){_undef.push_back(pair<bool,bool>(true,true));}
   ~CirMgr() {}

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { 
    int s=gid;
    auto it=GateMap.find(s);
    if(it==GateMap.end()){return 0;}
    return it->second;
    }
   // Member functions about circuit construction
   bool readCircuit(const string&);
   void connect();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void writeAag(ostream&) const;
   void satRAR() ;
   void satRARtest() ;
   

   CirGate* DFS(CirGate* p) const;
   CirGate* writeDFS(CirGate* p,ostream& outfile) const;

private:
vector<CirGate*>           _pilist;
vector<CirGate*>           _polist;
vector<CirGate*>          _aiglist;
vector<int>                      _floatlist;
vector<pair<bool,bool>>                       _undef;
mutable vector< string>                printList;
int                             M;
int                             I;
int                             O;
int                             A;
mutable int                             flAIG;
CirGate*                            const0;
map<int,CirGate*>       GateMap;
void readHeader(string header);
void readInput(string input,int l);
void readOutput(string output,int l);
void readAig(string Aig,int l);
void readSymbol(string symbol);
void resetprint() const;
void findAllgd(int, vector<pair<int, int>>&, map<int, int>&); // (Id, 0or1)
void compareTwogds(vector<pair<int, int>>&, vector<pair<int, int>>&);
void traverseWt(int, map<int, int>&, bool, bool&);
bool combineMAs(map<int, int>&, map<int, int>&);
vector<int> isConflict(map<int, int>&, map<int, int>&);
static bool mysort(CirGate const *p1,CirGate const* p2);
};

#endif // CIR_MGR_H
