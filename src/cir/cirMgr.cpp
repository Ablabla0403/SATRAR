/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "../sat/sat.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   const0=new CirConstGate();
   ifstream f(fileName.c_str());
   if(!f.is_open()){cout<<"Cannot open design \""<<fileName.c_str()<<"\"!!"<<endl;return false;}
   string line;
   getline(f,line);
   readHeader(line);
   for (int i=0;i<M;++i){
      _undef.push_back(make_pair(false,false));
   }   
   for(int i=0;i<I;i++){
      getline(f,line);
      readInput(line,i+2);
   }

   for(int i=0;i<O;i++){
      getline(f,line);
      readOutput(line,i+2+I);
   }
   for(int i=0;i<A;i++){
      getline(f,line);
      readAig(line,i+2+I+O);
   }
   while(getline(f,line)){
      if(line=="c"){break;}
      readSymbol(line);
   }
   connect();
   return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout<<endl;
   cout<<"Circuit Statistics"<<endl;
   cout<<"=================="<<endl;
   cout<<"  PI "<<setw(11)<<_pilist.size()<<endl;
   cout<<"  PO "<<setw(11)<<_polist.size()<<endl;
   cout<<"  AIG"<<setw(11)<<_aiglist.size()<<endl;
   cout<<"------------------"<<endl;
   cout<<"  Total"<<setw(9)<<_pilist.size()+_polist.size()+_aiglist.size()<<endl;
}

void
CirMgr::printNetlist() const
{
   for(unsigned i=0;i<_polist.size();++i){
     DFS(_polist[i]);
   }
   resetprint();
   cout<<endl;
   for(unsigned i=0;i<printList.size();++i){
      cout<<"["<<i<<"] "<<printList[i]<<endl;
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit: ";
   for(unsigned i=0;i<_pilist.size();i++){
      if(i!=_pilist.size()-1)cout<<_pilist[i]->getID()<<" ";
      else{cout<<_pilist[i]->getID();}
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit: ";
   for(unsigned i=0;i<_polist.size();i++){
      if(i!=_polist.size()-1)cout<<_polist[i]->getID()<<" ";
      else{cout<<_polist[i]->getID();}
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   if(_floatlist.size()==0){}
   else{cout<<"Gates with floating fanin(s): ";
   // sort(_floatlist.begin(),_floatlist.end(),mysort);
   for(unsigned i=0;i<_floatlist.size();++i){
      cout<<_floatlist[i]<<" ";
   }
   cout<<endl;
   }
   bool flag=false;
   for(unsigned i=0;i<_undef.size();++i){
      if(_undef[i].first==1 && _undef[i].second==0){
         if(!flag){cout<<"Gates defined but not used  : ";flag=true;}
         cout<<i<<" ";flAIG++;}
   }
   if(flag)cout<<endl;
}

void
CirMgr::writeAag(ostream& outfile) const
{
   outfile<<"aag "<<M<<" "<<I<<" 0 "<<O<<" "<<A-flAIG<<endl;
   for(unsigned i=0;i<_pilist.size();++i){
      outfile<<_pilist[i]->getID()*2<<endl;
   }
   for(unsigned i=0;i<_polist.size();++i){
      if(_polist[i]->getInv0()){outfile<<_polist[i]->getFanin0()->getID()*2+1<<endl;}
      else{outfile<<_polist[i]->getFanin0()->getID()*2<<endl;}
   }
   for(unsigned i=0;i<_polist.size();++i){
      writeDFS(_polist[i],outfile);
   }
   resetprint();
   for(unsigned i=0;i<_pilist.size();++i){
      if(_pilist[i]->getSymbol()!=""){outfile<<"i"<<i<<" "<<_pilist[i]->getSymbol()<<endl;}
   }
   for(unsigned i=0;i<_polist.size();++i){
      if(_polist[i]->getSymbol()!=""){outfile<<"o"<<i<<" "<<_polist[i]->getSymbol()<<endl;}
   }
   outfile<<"c"<<endl;
   outfile<<"AAG file output by Bing-Jia Chen."<<endl;
}

/**************************************************************/
/*   class CirMgr private member functions for circuit construction   */
/**************************************************************/
void CirMgr::readHeader(string header)
{
   istringstream iss(header);
   stringstream ss;
   string s;
   for (int i=0 ; i<6 ; i++){
      getline(iss,s,' ');
      if(i==1){
         ss<<s;
         ss>>M;
      }
      else if(i==2){
         ss<<s;
         ss>>I;
      }
      else if(i==4){
         ss<<s;
         ss>>O;
      }
      else if(i==5){
         ss<<s;
         ss>>A;
      }
      ss.str("");
      ss.clear();
   }
}
void CirMgr::readInput(string input,int l)
{
   stringstream ss;
   int ID=0;
   ss<<input;
   ss>>ID;
   CirGate* pi=new CirPiGate(ID/2,l);
   GateMap[ID/2]=pi;
   _undef[ID/2].first=true;
   _pilist.push_back(pi);
   ss.str("");
   ss.clear();
}
void CirMgr::readOutput(string output,int l)
{
   stringstream ss;
   int fanin=0;
   ss<<output;
   ss>>fanin;
   bool inv=(fanin%2==1);
   _undef[fanin/2].second=true;
   CirGate* po=new CirPoGate(fanin/2,M+l-I-1,inv,l);
   GateMap[M+l-I-1]=po;
   _polist.push_back(po);
   ss.str("");
   ss.clear();
}
void CirMgr::readAig(string Aig,int l)
{
   istringstream iss(Aig);
   stringstream ss;
   string s;
   int ID,fanin0,fanin1;
   getline(iss,s,' ');
   ss<<s;
   ss>>ID;
   ss.str("");
   ss.clear();
   getline(iss,s,' ');
   ss<<s;
   ss>>fanin0;
   ss.str("");
   ss.clear();
   getline(iss,s,' ');
   ss<<s;
   ss>>fanin1;
   ss.str("");
   ss.clear();
   bool i1=(fanin0%2==1);
   bool i2=(fanin1%2==1);
   _undef[ID/2].first=true;
   if(fanin0/2!=0){_undef[fanin0/2].second=true;}
   if(fanin1/2!=0){_undef[fanin1/2].second=true;}
   CirGate* aig=new CirAigGate(fanin0/2,fanin1/2,ID/2,i1,i2,l);
   GateMap[ID/2]=aig;
   _aiglist.push_back(aig);
}
void CirMgr::readSymbol(string symbol)
{
   istringstream iss(symbol);
   stringstream ss;
   string s;
   int index;
   getline(iss,s,' ');
   if(s[0]=='i'){
      string  s1=s1.assign(s,1,s.size()-1);
      ss<<s1;
      ss>>index;
      getline(iss,s,' ');
      _pilist[index]->setSymbol(s);  
   }
   else if(s[0]=='o'){
      string  s1=s1.assign(s,1,s.size()-1);
      ss<<s1;
      ss>>index;
      getline(iss,s,' ');
      _polist[index]->setSymbol(s);  
   }
   ss.str("");
   ss.clear();
}


void CirMgr::connect()
{
   map<int,CirGate*>::iterator it;
   for (it=GateMap.begin();it!=GateMap.end();++it){
      if(it->second->getTypeStr()=="Pi"){}
      else if(it->second->getTypeStr()=="Po"){
         pair<int,bool>check0(it->second->getPair0());
         if(check0.first==0){it->second->setFanin0(const0,check0.second);const0->pushFanout(it->second);}
         else{
            if(GateMap[check0.first]==0){
               CirGate* p=new CirUndefGate(check0.first);
                it->second->setFanin0(p,check0.second);
                p->pushFanout(it->second);
                GateMap[check0.first]=p;
               _floatlist.push_back(check0.first);}
            else{it->second->setFanin0(GateMap[check0.first],check0.second);GateMap[check0.first]->pushFanout(it->second);}
            }
      }
      else if(it->second->getTypeStr()=="Aig"){
         pair<int,bool>check0(it->second->getPair0());
         pair<int,bool>check1(it->second->getPair1());
         if(check0.first==0){it->second->setFanin0(const0,check0.second);const0->pushFanout(it->second);}
         else{
            if(GateMap[check0.first]==0){
               CirGate* p=new CirUndefGate(check0.first);
                it->second->setFanin0(p,check0.second);
                p->pushFanout(it->second);
                GateMap[check0.first]=p;
               _floatlist.push_back(check0.first);}
            else{it->second->setFanin0(GateMap[check0.first],check0.second);GateMap[check0.first]->pushFanout(it->second);}
            }
         if(check1.first==0){it->second->setFanin1(const0,check1.second);const0->pushFanout(it->second);}
         else{
            if(GateMap[check1.first]==0){
               CirGate* p=new CirUndefGate(check1.first);
                it->second->setFanin1(p,check1.second);
                p->pushFanout(it->second);
               if(GateMap[check0.first]!=0)_floatlist.push_back(check1.first);}
            else{it->second->setFanin1(GateMap[check1.first],check1.second);GateMap[check1.first]->pushFanout(it->second);}
            }
         
      }
   }
   
}

CirGate* CirMgr::DFS(CirGate* p) const
{
   stringstream ss;
   string ID;
   if(p==0){return 0;}
   if(p->checkprint()){return 0;}
   DFS(p->getFanin0());
   DFS(p->getFanin1());
   if(p->checkprint()){return 0;}
   if(p->getTypeStr()=="Pi"){
      string str;
      ss<<p->getID();
      ss>>ID;
      if(p->getSymbol()!=""){str="PI  "+ID+" ("+p->getSymbol()+")";printList.push_back(str);}
      else{str="PI  "+ID;printList.push_back("PI  "+ID);}
      p->setprint();
      ss.str("");
      ss.clear();
      // return 0;
   }
   if(p->getTypeStr()=="Aig"){
      string str="";
      string ID;
      ss<<p->getID();
      ss>>ID;
      ss.str("");
      ss.clear();
      str="AIG "+ID;
      ss<<p->getFanin0()->getID();
      ss>>ID;
      if(p->getFanin0()->getTypeStr()=="UNDEF" && p->getInv0()){str=str+" *!"+ID;}
      else if(p->getInv0()){str=str+" !"+ID;}
      else if(p->getFanin0()->getTypeStr()=="UNDEF"){str=str+" !"+ID;}
      else{str=str+" "+ID;}
      ss.str("");
      ss.clear();
      ss<<p->getFanin1()->getID();
      ss>>ID;
      if(p->getFanin1()->getTypeStr()=="UNDEF" && p->getInv1()){str=str+" *!"+ID;}
      else if(p->getInv1()){str=str+" !"+ID;}
      else if(p->getFanin1()->getTypeStr()=="UNDEF"){str=str+" !"+ID;}
      else{str=str+" "+ID;}
      ss.str("");
      ss.clear();
      // cout<<"["<<i<<"] "<<str<<endl;
      printList.push_back(str);
      p->setprint();
      
      // return 0;
   }
   if(p->getTypeStr()=="Po"){
      string str="";
      ss<<p->getID();
      ss>>ID;
      ss.str("");
      ss.clear();
      str="PO  "+ID;
      ss<<p->getFanin0()->getID();
      ss>>ID;
      if(p->getFanin0()->getTypeStr()=="UNDEF" && p->getInv0()){str=str+" *!"+ID;}
      else if(p->getInv0()){str=str+" !"+ID;}
      else if(p->getFanin0()->getTypeStr()=="UNDEF"){str=str+" !"+ID;}
      else{str=str+" "+ID;}
      if(p->getSymbol()!=""){str+=" ("+p->getSymbol()+")";}
      // cout<<"["<<i<<"] "<<str<<endl;
      printList.push_back(str);
      ss.str("");
      ss.clear();
      p->setprint();
      // return 0;
   }
   if(p->getTypeStr()=="const"){
      printList.push_back("CONST0");
      // cout<<"["<<i<<"] "<<"CONST0"<<endl;
      p->setprint();
      // return 0;
   }
   if(p->getTypeStr()=="UNDEF"){
      // return 0;
   }

   }

void CirMgr::resetprint()const
{
   map<int,CirGate*>::iterator it;
   for(auto it=GateMap.begin();it!=GateMap.end();++it){
      it->second->resetprint();
   }
   const0->resetprint();
}

CirGate* CirMgr::writeDFS(CirGate* p,ostream& outfile)const{
   if(p==0){return 0;}
   if(p->checkprint()){return 0;}
   writeDFS(p->getFanin0(),outfile);
   writeDFS(p->getFanin1(),outfile);
   if(p->getTypeStr()=="Aig"){
      int F0,F1;
      if(p->getInv0()){F0=p->getFanin0()->getID()*2+1;}
      else{F0=p->getFanin0()->getID()*2;}
      if(p->getInv1()){F1=p->getFanin1()->getID()*2+1;}
      else{F1=p->getFanin1()->getID()*2;}
      p->setprint();
      outfile<<p->getID()*2<<" "<<F0<<" "<<F1<<endl;
   }
}

bool CirMgr::mysort(CirGate const* p1,CirGate const* p2){
   return (p1->getID()<p2->getID());
}


void CirMgr::traverseWt(int wt, map<int, int>& excite, bool iswt, bool& conflict) {
   CirGate* tmp;
   if (conflict) {
      return;
   }
   if (GateMap[wt]->getTypeStr() == "Aig") {
      // input
      // situation of fanin0
      if (excite[wt] == 1) { // output is 1, two input are all 1
         if (GateMap[wt]->getInv0() == false) {
            if (excite[GateMap[wt]->getFanin0()->getID()] == 0) {
               conflict = true;
               return;
            }
            excite[GateMap[wt]->getFanin0()->getID()] = 1;
         } else {
            if (excite[GateMap[wt]->getFanin0()->getID()] == 1) {
               conflict = true;
               return;
            }
            excite[GateMap[wt]->getFanin0()->getID()] = 0;
         }
         traverseWt(GateMap[wt]->getFanin0()->getID(), excite, false, conflict);
      } 
      else { // output is 0, one input is 0 (if the other one is 1)
         if(excite[GateMap[wt]->getFanin1()->getID()] == 1 && !GateMap[wt]->getInv1()
            || excite[GateMap[wt]->getFanin1()->getID()] == 0 && GateMap[wt]->getInv1()) {
            //no inverter
            if (GateMap[wt]->getInv0() == false) {
               if (excite[GateMap[wt]->getFanin0()->getID()] == 1) {
                  conflict = true;
               }
               excite[GateMap[wt]->getFanin0()->getID()] = 0;
            } else {
               if (excite[GateMap[wt]->getFanin0()->getID()] == 0) {
                  conflict = true;
               }
               excite[GateMap[wt]->getFanin0()->getID()] = 1;
            }
           
            traverseWt(GateMap[wt]->getFanin0()->getID(), excite, false, conflict);
         }
      }
      // situation of fanin1
      if (excite[wt] == 1) {
         if (GateMap[wt]->getInv1() == false) {
            if(excite[GateMap[wt]->getFanin1()->getID()] == 0) {
               conflict = true;
               return;
            }
            excite[GateMap[wt]->getFanin1()->getID()] = 1;
         } else {
            if(excite[GateMap[wt]->getFanin1()->getID()] == 1) {
               conflict = true;
               return;
            }
            excite[GateMap[wt]->getFanin1()->getID()] = 0;
         }
         traverseWt(GateMap[wt]->getFanin1()->getID(), excite, false,conflict);
      } 
      else {
         if(excite[GateMap[wt]->getFanin0()->getID()] == 1 && !GateMap[wt]->getInv0()
            || excite[GateMap[wt]->getFanin0()->getID()] == 0 && GateMap[wt]->getInv0()) {
            // no inverter
            if (GateMap[wt]->getInv1() == false) {
               if(excite[GateMap[wt]->getFanin1()->getID()] == 1) {
                  conflict = true;
                  return;
               }
               excite[GateMap[wt]->getFanin1()->getID()] = 0;
            } else {
               if(excite[GateMap[wt]->getFanin1()->getID()] == 0) {
                  conflict = true;
                  return;
               }
               excite[GateMap[wt]->getFanin1()->getID()] = 1;
            }
            
            traverseWt(GateMap[wt]->getFanin1()->getID(), excite, false, conflict);
         }
      }
   }
   // output
   if(!iswt) {
      for (size_t i=0; i<GateMap[wt]->getFanout().size(); ++i) {
         tmp = GateMap[wt]->getFanout()[i];
         if (excite[tmp->getID()] != 2) {
            continue;
         } else if(tmp->getTypeStr() == "Aig") {
            // situations of false fanOut (fanin is 0)
            if (excite[tmp->getFanin0()->getID()] == 1 && tmp->getInv0()) {
               if (excite[tmp->getID()] == 1) {
                  conflict = true;
                  return;
               }
               excite[tmp->getID()] = 0;
               traverseWt(tmp->getID(), excite, false, conflict);
            } else if(excite[tmp->getFanin0()->getID()] == 0 && !tmp->getInv0()){
               if (excite[tmp->getID()] == 1) {
                  conflict = true;
                  return;
               }
               excite[tmp->getID()] = 0;
               traverseWt(tmp->getID(), excite, false, conflict);
            } else if(excite[tmp->getFanin1()->getID()] == 1 && tmp->getInv1()){
               if (excite[tmp->getID()] == 1) {
                  conflict = true;
                  return;
               }
               excite[tmp->getID()] = 0;
               traverseWt(tmp->getID(), excite, false, conflict);
            } else if(excite[tmp->getFanin1()->getID()] == 0 && !tmp->getInv1()){
               if (excite[tmp->getID()] == 1) {
                  conflict = true;
                  return;
               }
               excite[tmp->getID()] = 0;
               traverseWt(tmp->getID(), excite, false, conflict);
            }
            // situations of true fanOut(two inputs are 1)
            if ((excite[tmp->getFanin0()->getID()] == 1 && !tmp->getInv0()) 
                  || (excite[tmp->getFanin0()->getID()] == 0 && tmp->getInv0())) {
               if ((excite[tmp->getFanin1()->getID()] == 1 && !tmp->getInv1())
                     || (excite[tmp->getFanin1()->getID()] == 0 && tmp->getInv1())) {
                  if (excite[tmp->getID()] == 0) {
                     conflict = true;
                     return;
                  }
                  excite[tmp->getID()] = 1;
                  traverseWt(tmp->getID(), excite, false, conflict);
               }
            }
         } 
      }
   }
}

vector<int> CirMgr::isConflict(map<int, int>& MA1, map<int, int>& MA2) {
   vector<int> result;
   for (auto it=GateMap.begin(); it!=GateMap.end(); ++it){
      if (MA1[it->first] == 1 && MA2[it->first] == 0) {
         result.push_back(it->first);
      } else if (MA1[it->first] == 1 && MA2[it->first] == 0) {
         result.push_back(it->first);
      }
   }
   return result;
}

bool CirMgr::combineMAs(map<int, int>& MA1, map<int, int>& MA2) {
   for (auto it=MA1.begin(); it!=MA1.end(); it++) {
      if ((MA1[it->first] == 1 && MA2[it->first] == 0)) {
         return false;
      } 
      else if (MA1[it->first] == 0 && MA2[it->first] == 1) {
         return false;
      } 
      else if (MA1[it->first] == 1 || MA2[it->first] == 1) {
         MA1[it->first] = 1;
      }
      else if (MA1[it->first] == 0 || MA2[it->first] == 0) {
         MA1[it->first] = 0;
      } 
      else {
         MA1[it->first] = 2;
      }
   }
   return true;
}

void CirMgr::compareTwogds(vector<pair<int, int>>& gds1, vector<pair<int,int>>& gds2) {
   int i = 0, tmp =0;
   while (i < gds1.size()) {
      for (auto it=gds2.begin(); it!=gds2.end(); it++) {
         if(gds1[i].first == it->first) {
            tmp = 1;
         }
      }
      if (tmp == 0) {
         gds1.erase(gds1.begin()+i);
      } else {
         i++;
         tmp = 0;
      }
   }
}

class Gate
{
public:
   Gate(unsigned i = 0): _gid(i) {}
   ~Gate() {}

   Var getVar() const { return _var; }
   void setVar(const Var& v) { _var = v; }

private:
   unsigned   _gid;  // for debugging purpose...
   Var        _var;
};

void genAigModel(SatSolver& s, vector<Gate*>& gates, map<int, CirGate *>& GateMap) {
   // Allocate and record variables; No Var ID for POs
   for (size_t i=1; i<gates.size(); ++i) {
      Var v = s.newVar();
      gates[i]->setVar(v);
   }

   for (auto it=GateMap.begin(); it!=GateMap.end(); it++) {
      if (it->second->getTypeStr() == "Aig"){
         s.addAigCNF(gates[it->first]->getVar(), gates[it->second->getFanin0()->getID()]->getVar(), it->second->getInv0(),
                  gates[it->second->getFanin1()->getID()]->getVar(), it->second->getInv1());
      }  
   }
}

// void CirMgr::satRARtest() {
//    map<int, int> excite, propagation, MAgd;      // (0:neg 1:pos: 2:free)
//    vector<pair<int, int>> gds1, gds2; // (fanoutId, fanin0 or fanin1) gds1 for excitation, gds2 for checking others
//    vector<int> redundant;
//    bool conflict = false;
//    for (auto it=GateMap.begin();it!=GateMap.end();++it){
//       excite[it->first] = 2;
//       propagation[it->first] = 2;
//    }

//    for (auto it=GateMap.begin(); it!=GateMap.end(); ++it){
//       if (it->second->getTypeStr() == "Aig" || it->second->getTypeStr() == "Pi") {
//          cout << "(" << it->first << ")" << "\n";
//          excite[it->first] = 1;
//          traverseWt(it->first, excite, true, conflict);
//          if (conflict) {
//             conflict = false;
//             cout << "CONFLICT!!!\n";
//          }
//          if (it->second->getTypeStr() == "Aig") {
//             if (it->second->getFanout()[0]->getTypeStr() == "Aig") { // construct gds for first fanout
//                if(it->second->getFanout()[0]->getFanin0()->getID() == it->first) {
//                   gds1.push_back(pair<int, int>(it->second->getFanout()[0]->getID(), 0));
//                } else {
//                   gds1.push_back(pair<int, int>(it->second->getFanout()[0]->getID(), 1));  
//                }
//             }
//             findAllgd(it->second->getFanout()[0]->getID(), gds1);
//             if (it->second->getFanout().size() > 1) { // check other fanouts and compare with gds1, then get final gds1
//                for (size_t i=1; i<it->second->getFanout().size(); ++i) {
//                   if(it->second->getFanout()[i]->getTypeStr() == "Aig") {
//                      if(it->second->getFanout()[i]->getFanin0()->getID() == it->first) {
//                         gds2.push_back(pair<int, int>(it->second->getFanout()[i]->getID(), 0));
//                      } else {
//                         gds2.push_back(pair<int, int>(it->second->getFanout()[i]->getID(), 1));  
//                      }
//                   }
//                   findAllgd(it->second->getFanout()[i]->getID(), gds2, check_gd);
//                }
//                compareTwogds(gds1, gds2);
//                gds2.clear();
//                if(gds1.size() == 0) {
//                   gds1.clear();  
//                }
//             }
//             // for(int j=0; j<gds1.size(); ++j) {
//             //    cout << "(" << gds1[j].first << "  " << gds1[j].second << ")\n" ;
//             // }
//             for (size_t i=0; i<gds1.size(); ++i) {
//                if(gds1[i].second == 0){
//                   if (GateMap[gds1[i].first]->getInv1() == false) {
//                      propagation[GateMap[gds1[i].first] -> getFanin1()->getID()] = 1;
//                   } else {
//                      propagation[GateMap[gds1[i].first] -> getFanin1()->getID()] = 0;
//                   }
//                   traverseWt(GateMap[gds1[i].first] -> getFanin1()->getID(), propagation, false, conflict);
//                   if (conflict) {
//                      conflict = false;
//                      cout << "CONFLICT!!!\n";
//                   }
//                } else {
//                   if (GateMap[gds1[i].first]->getInv0() == false) {
//                      propagation[GateMap[gds1[i].first] -> getFanin0()->getID()] = 1;
//                   } else {
//                      propagation[GateMap[gds1[i].first] -> getFanin0()->getID()] = 0;
//                   }
//                   traverseWt(GateMap[gds1[i].first] -> getFanin0()->getID(), propagation, false, conflict);
//                   if (conflict) {
//                      conflict = false;
//                      cout << "CONFLICT!!!\n";
//                   }
//                }
//             }
//             if (!combineMAs(excite, propagation)) {  // combine two MAs
//                cout << "CONFLICT!!!\n";
//             }
//             // for (auto itt=excite.begin(); itt!=excite.end(); itt++) {
//             //    cout << itt->first << "  " << itt->second << "\n";
//             // }
//             for (size_t i=0; i<gds1.size(); ++i) {
//                propagation[gds1[i].first] = 1;
//                traverseWt(gds1[i].first, MAgd, true, conflict);
//                if (conflict) {
//                   conflict = false;
//                   cout << "CONFLICT!!!\n";
//                }
//                if(!combineMAs(excite, MAgd)) {
//                   cout << "Gd: " << gds1[i].first << " CONFLICT!!!\n";
//                }
//                for (auto it2=GateMap.begin(); it2!=GateMap.end(); it2++){
//                   MAgd[it2->first] = 2;
//                }
//             }
//             redundant.clear();
//             for (auto it2=GateMap.begin(); it2!=GateMap.end(); it2++){
//                propagation[it2->first] = 2;
//             }
//             conflict = false;
//             gds1.clear();
//          }
//          for (auto it2=GateMap.begin(); it2!=GateMap.end(); it2++){
//             // cout << it2->first << " " << excite[it2->first] << "\n";
//             excite[it2->first] = 2;
//             propagation[it2->first] = 2;
//          }
//       }
//       cout << "\n";
//    }
// }

void clear_MA(map<int, int>& MA) {
   for (auto it=MA.begin();it!=MA.end();++it){
      it->second = 2;
   }
}

void CirMgr:: findAllgd(int Id, vector<pair<int,int>>& gds, map<int, int>& check_gd) {
   for (size_t i=0; i<GateMap[Id]->getFanout().size(); ++i) {
      if (GateMap[Id]->getFanout()[i]->getTypeStr() == "Aig" && check_gd[GateMap[Id]->getFanout()[i]->getID()] == 2) {
         if (GateMap[Id]->getFanout()[i]->getFanin0()->getID() == Id) {
            check_gd[GateMap[Id]->getFanout()[i]->getID()] = 1;
            gds.push_back(pair<int, int>(GateMap[Id]->getFanout()[i]->getID(), 0));
         } else {
            check_gd[GateMap[Id]->getFanout()[i]->getID()] = 1;
            gds.push_back(pair<int, int>(GateMap[Id]->getFanout()[i]->getID(), 1));
         }
         findAllgd(GateMap[Id]->getFanout()[i]->getID(), gds, check_gd);
      }
   }
}

void CirMgr::satRAR() {

   // write aig circuit into cnf 
   vector<Gate* > gates;
   vector<pair<int,int>> gds1, gds2, gds3, gds4;
   map<int, int> MA_wt, MA_gd, MA_wt2, check_gd; // MA_wt2 for the second decision
   bool result_wt = true, result_gd = true, isConflict = false, result_wt2;
   SatSolver solver;
   solver.initialize();

   for(int i=0; i<GateMap.size()+1; ++i) {
      gates.push_back(new Gate(i));
   }
   genAigModel(solver, gates, GateMap);

   for (auto it=GateMap.begin();it!=GateMap.end();++it){
      MA_gd[it->first] = 2;
      MA_wt[it->first] = 2;
      MA_wt2[it->first] = 2;
      check_gd[it->first] = 2;
   }

   // Clear assumptions
   solver.assumeRelease();  

   int count_tar = 0, count_alt = 0;
   vector<int> fanout;
   vector<Var> vv;
   vector<bool> vb;
   bool findwire = false, findgate = false;
   // make assumptions
   int id;
   int test = 0;

   cout << GateMap.size() << "\n";

   for (auto itt=GateMap.begin();itt!=GateMap.end();++itt){
      
      // cout << "type: " << itt->second->getTypeStr() << "\n";

      if(itt->second->getTypeStr() == "Aig") {

         // for (size_t l=0; l<itt->second->getFanout().size(); l++) {
         //    if(itt->second->getFanout()[l]->getTypeStr() == "PO") {

         //    }
         // }

         fanout.clear();
         gds1.clear();
         gds2.clear();
         vv.clear();
         vb.clear();
         clear_MA(MA_wt);

         id = itt->first;
         count_tar ++;
         cout << "id: " << id << " count_tar: " << count_tar << "\n";
      
         vv.push_back(gates[id]->getVar());
         vb.push_back(true);

         MA_wt[gates[id]->getVar()] = true;

         auto it = GateMap[id];
         // find all fanout
         

         // // calculate all Gds   

         // if (id == 344 || id == 345) {
         //    for (size_t k=0; k<it->getFanout().size(); k++) {
         //       cout << "fsdfdsf " << it->getFanout()[k]->getID() << "\n";
         //    }
         // }
         
         if (it->getFanout()[0]->getTypeStr() == "Aig") { // construct gds for first fanout
            if(it->getFanout()[0]->getFanin0()->getID() == id) {
               gds1.push_back(pair<int, int>(it->getFanout()[0]->getID(), 0));
            } else {
               gds1.push_back(pair<int, int>(it->getFanout()[0]->getID(), 1));  
            }
         }

         findAllgd(it->getFanout()[0]->getID(), gds1, check_gd);
         clear_MA(check_gd);
         if (it->getFanout().size() > 1) { // check other fanouts and compare with gds1, then get final gds1
            for (size_t i=1; i<it->getFanout().size(); ++i) {
               if(it->getFanout()[i]->getTypeStr() == "Aig") {
                  if(it->getFanout()[i]->getFanin0()->getID() == id) {
                     gds2.push_back(pair<int, int>(it->getFanout()[i]->getID(), 0));
                  } else {
                     gds2.push_back(pair<int, int>(it->getFanout()[i]->getID(), 1));  
                  }
               }
               findAllgd(it->getFanout()[i]->getID(), gds2, check_gd);
               clear_MA(check_gd);
            }
            compareTwogds(gds1, gds2);
            gds2.clear();
            if(gds1.size() == 0) {
               gds1.clear();  
            }
         }

         // for (size_t k=0; k<gds1.size(); k++) {
         //    cout << "  gds  " << gds1[k].first << "\n";
         // }


         for (size_t i=0; i<gds1.size(); ++i) {
            fanout.push_back(gds1[i].first);
         }
         

         // excitation
         vector<int> empty;

         for (auto it2=gds1.begin(); it2!=gds1.end(); it2++) {
            if (it2->second == 1) {
               vv.push_back(gates[GateMap[it2->first]->getFanin0()->getID()]->getVar());
               if (GateMap[it2->first]->getInv0()) vb.push_back(false);
               else vb.push_back(true);

            } else {
               vv.push_back(gates[GateMap[it2->first]->getFanin1()->getID()]->getVar());
               if (GateMap[it2->first]->getInv1()) vb.push_back(false);
               else vb.push_back(true);
            }

            MA_wt[vv[vv.size()-1]] = vb[vb.size()-1];
         }

         // for (size_t j=0; j<vv.size(); j++) {
         //    cout << vv[j] << "  " << vb[j] << "\n";
         // }

         solver.assumeVec(vv, vb);
         // cout << gds1.size() << "   " <<  vv.size() << "   " << vb.size() << "\n";

         result_wt = solver.assumpRARSolve(MA_wt, fanout);
         // solver.printStats();
         // for (auto it2=GateMap.begin();it2!=GateMap.end();++it2){
         //    cout << "id: " << it2->first << " value: " << MA_wt[it2->first] << "\n";
         // }

         solver.assumeRelease();
         solver.resetAssign();

         // cout << (result_wt? "SAT" : "UNSAT") << endl;

         vv.clear();
         vb.clear();
         solver.assumeRelease();
         solver.resetAssign();

         // for each Gd
         for (auto it2=gds1.begin(); it2!=gds1.end(); it2++) {
            // cout << "===========================\n";
            // cout << "Gd_id: " << it2->first << "\n";
            if (findwire || findgate) {
               findwire = false;
               findgate = false;
               break;
            }

            solver.assumeRelease();

            vv.push_back(gates[it2->first]->getVar());
            vb.push_back(true);
            
            MA_gd[vv[0]] = vb[0];
            solver.assumeVec(vv, vb);
            result_gd = solver.assumpRARSolve(MA_gd, empty);
            // for (auto it3=GateMap.begin();it3!=GateMap.end();++it3){
            //    cout << "id: " << it3->first << " value: " << MA_gd[it3->first] << "\n";
            // }
            solver.assumeRelease();
            solver.resetAssign();

            // for each wire in MA_wt but not in MA_gd

            for (auto it3=GateMap.begin();it3!=GateMap.end(); ++it3){
               if ((MA_wt[it3->first] != 2) && (MA_gd[it3->first] == 2)) {
                  // for Gd
                  vv.push_back(gates[it2->first]->getVar());
                  vb.push_back(true);
                  // for decision
                  vv.push_back(gates[it3->first]->getVar());
                  vb.push_back(MA_wt[it3->first]);

                  if (it->getFanout()[0]->getTypeStr() == "Aig") { // construct gds for first fanout
                     if(it->getFanout()[0]->getFanin0()->getID() == it3->first) {
                        gds3.push_back(pair<int, int>(it->getFanout()[0]->getID(), 0));
                     } else {
                        gds3.push_back(pair<int, int>(it->getFanout()[0]->getID(), 1));  
                     }
                  }

                  findAllgd(it->getFanout()[0]->getID(), gds3, check_gd);
                  clear_MA(check_gd);
                  if (it->getFanout().size() > 1) { // check other fanouts and compare with gds1, then get final gds1
                     for (size_t i=1; i<it->getFanout().size(); ++i) {
                        if(it->getFanout()[i]->getTypeStr() == "Aig") {
                           if(it->getFanout()[i]->getFanin0()->getID() == it3->first) {
                              gds4.push_back(pair<int, int>(it->getFanout()[i]->getID(), 0));
                           } else {
                              gds4.push_back(pair<int, int>(it->getFanout()[i]->getID(), 1));  
                           }
                        }
                        findAllgd(it->getFanout()[i]->getID(), gds4, check_gd);
                        clear_MA(check_gd);
                     }
                     compareTwogds(gds3, gds4);
                     gds4.clear();
                     if(gds3.size() == 0) {
                        gds3.clear();  
                     }
                  }

                  for (auto it4=gds3.begin(); it4!=gds3.end(); it4++) {
                     if (it4->second == 1) {
                        vv.push_back(gates[GateMap[it4->first]->getFanin0()->getID()]->getVar());
                        if (GateMap[it4->first]->getInv0()) vb.push_back(false);
                        else vb.push_back(true);

                     } else {
                        vv.push_back(gates[GateMap[it4->first]->getFanin1()->getID()]->getVar());
                        if (GateMap[it4->first]->getInv1()) vb.push_back(false);
                        else vb.push_back(true);
                     }

                     MA_gd[vv[vv.size()-1]] = vb[vb.size()-1];
                  }

                   for (size_t m=0; m<gds3.size(); ++m) {
                     fanout.push_back(gds1[m].first);
                  }


                  
                  solver.assumeVec(vv, vb);
                  // cout << "select ID " << it3->first << " value " << vb[0] << "\n";

                  result_gd = solver.assumpRARSolve(MA_gd, fanout);
                  fanout.clear();
                  // for (auto it4=GateMap.begin();it4!=GateMap.end();++it4){
                  //    cout << "id: " << it4->first << " value: " << MA_gd[it4->first] << "\n";
                  // }
                  if (!result_gd) {
                     if ( (it3->first != id) || ((GateMap[it2->first]->getFanin0()->getID() != it3->first) 
                                             && (GateMap[it2->first]->getFanin1()->getID() != it3->first))){
                        cout << "alternative wire: " << it3->first << " -> " << it2->first << "  " << count_alt << "\n"; 
                        findwire = true;
                        count_alt ++;
                     }
                  }
                  else {
                     for (auto it4=GateMap.begin();it4!=GateMap.end();++it4){
                        if((MA_gd[it4->first] != 2) && (MA_wt[it4->first] != 2) && (MA_wt[it4->first] != MA_gd[it4->first]) && it3->first != id && it4->first != id) {
                           cout << "alternative gate: " << it3->first  << " & " << it4->first << " -> " << it2->first << "\n"; 
                           isConflict = true;
                           count_alt ++;
                           // cout << "count_alt: " << count_alt << "\n";
                           findgate = true;
                           break;
                        }
                        // if(!isConflict) {
                        //    for (auto it5=GateMap.begin(); it5!=GateMap.end(); it5++) {
                        //       vv.clear();
                        //       vb.clear();S
                        //       solver.assumeRelease();
                        //       solver.resetAssign();
                        //       if ((MA_gd[it5->first] != 2) && (MA_wt[it5->first] == 2)) {
                        //          // for Gd
                        //          vv.push_back(gates[it2->first]->getVar());
                        //          vb.push_back(true);
                        //          // for decision of Wt
                        //          vv.push_back(gates[it3->first]->getVar());
                        //          vb.push_back(MA_wt[it3->first]);
                        //          // for decision of Gd
                        //          vv.push_back(gates[it3->first]->getVar());
                        //          vb.push_back(MA_wt[it3->first]);
                        //          solver.assumeVec(vv, vb);
                        //          result_wt2 = solver.assumpRARSolve(MA_wt2, empty);
                        //       }
                        //       for (auto it6=GateMap.begin();it6!=GateMap.end();++it6) {
                        //          if((MA_wt2[it6->first] != 2) && (MA_gd[it6->first] != 2) && (MA_wt[it6->first] != MA_gd[it6->first])) {
                        //             cout << "alternative multi-gate: " << it6->first << " & " <<it3->first  << " & " << it4->first << " -> " << it2->first << "\n"; 
                        //          }
                        //       }
                        //    }
                        // }
                        // isConflict = false;
                     }
                  }
            //       // cout << (result_gd? "SAT" : "UNSAT") << endl;

            //       //reset 
                  vv.clear();
                  vb.clear();
                  solver.assumeRelease();
                  solver.resetAssign();
                  
                  for (auto it4=GateMap.begin();it4!=GateMap.end();++it4){
                     MA_gd[it4->first] = 2;
                  }
                  if (findwire || findgate) {
                     break;
                  }
               }
            }
         }
      }
   }

   cout << "#tar: " << count_tar << " #alt: " << count_alt << "\n";
   cout << "test: " << test << "\n";
   cout << GateMap.size() << "\n";
}