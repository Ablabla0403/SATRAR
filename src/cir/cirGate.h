/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include<sstream>
#include "cirDef.h"

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
  public:
   CirGate():_fanin0(0),_fanin1(0),_inv0(0),_inv1(0),lineNo(0),symbol(""),isprint(0) {_fanout.clear();}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const { return type; }
   unsigned getLineNo() const { return lineNo; }
   void setSymbol(string str){symbol=str;}
   string getSymbol()const{return symbol;}
   void setLineNo(int No){lineNo=No;}
   void setType(string str){type=str;}
   CirGate* getFanin0()const{return _fanin0;}
   CirGate* getFanin1()const{return _fanin1;}
   void setFanin0(CirGate* p,bool i){_fanin0=p;_inv0=i;}
   void setFanin1(CirGate* p,bool i){_fanin1=p;_inv1=i;}
   bool getInv0()const{return _inv0;}
   bool getInv1()const{return _inv1;}
   void setprint()const{isprint=1;}
   void resetprint()const{isprint=0;}
   bool checkprint()const{return isprint;}
   // Printing functions
   virtual void printGate() const{} ;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
   CirGate* DFS(const CirGate* p,int level,int i,bool inv) const;
   CirGate* revDFS(const CirGate* p,int level,int i,bool inv) const;
   CirGate* rprint(const CirGate* p,int level)const;
   CirGate* revrprint(const CirGate*,int level)const;
   void pushFanout(CirGate* p){_fanout.push_back(p);}
   vector<CirGate*> getFanout()const{return _fanout;}
   //helper function
   virtual int getID() const{};
   virtual pair<int,bool> getPair0() const{};
   virtual pair<int,bool> getPair1() const{};
private:
string                             type;
string                             symbol;
unsigned                     lineNo;
vector<CirGate*>      _fanout;
CirGate*                       _fanin0;
CirGate*                      _fanin1;
bool                              _inv0;
bool                              _inv1;
mutable bool                              isprint;
protected:

};


class CirPiGate:public CirGate
{
  public:
      CirPiGate(int ID,int No):gateID(ID){setLineNo(No);setType("Pi");}
      ~CirPiGate(){}
      virtual void printGate()const{
        stringstream ss;
        string ID,No;
        ss<<gateID;
        ss>>ID;
        ss.str("");
        ss.clear();
        ss<<getLineNo();
        ss>>No;
        string str="";
        cout<<"=================================================="<<endl;
        if(getSymbol()==""){str="= PI(" + ID + "), line "+No;}
        else{str="= PI(" + ID + ")\""+getSymbol()+"\", line "+No;}
        cout<<setw(49)<<left<<str<<"="<<endl;
        cout<<"=================================================="<<endl;
        ss.str("");
        ss.clear();
      }
      virtual int getID()const{return gateID;}
      virtual pair<int,bool> getPair0() const{return pair<int,bool>(0,0);}
      virtual pair<int,bool> getPair1() const{return pair<int,bool>(0,0);}

  private:
  int                 gateID;

};

class CirPoGate:public CirGate
{
  public:
      CirPoGate(int f,int ID,bool i,int No):gateID(ID){
        setLineNo(No);
        setType("Po");
        _fanin0.first=f;
        _fanin0.second=i;
        }
      ~CirPoGate(){}
      virtual void printGate()const{
        stringstream ss;
        string ID,No;
        ss<<gateID;
        ss>>ID;
        ss.str("");
        ss.clear();
        ss<<getLineNo();
        ss>>No;
        string str="";
        cout<<"=================================================="<<endl;
        if(getSymbol()==""){str="= PO(" + ID + "), line "+No;}
        else{str="= PO(" + ID + ")\""+getSymbol()+"\", line "+No;}
        cout<<setw(49)<<left<<str<<"="<<endl;
        cout<<"=================================================="<<endl;
        ss.str("");
        ss.clear();
      }
      virtual int getID()const{return gateID;}
      virtual pair<int,bool> getPair0() const{ return _fanin0;}
      virtual pair<int,bool> getPair1() const{return pair<int,bool>(0,0);}
  private:
  int               gateID;
  pair<int,bool>        _fanin0;

};

class CirAigGate:public CirGate
{
  public:
      CirAigGate(int f0,int f1,int ID,bool i1,bool i2,int No):gateID(ID){
        setLineNo(No);
        setType("Aig");
        _fanin0.first=f0;
        _fanin0.second=i1;
        _fanin1.first=f1;
        _fanin1.second=i2;
        }
      ~CirAigGate(){}
      virtual void printGate()const{
        stringstream ss;
        string ID,No;
        ss<<gateID;
        ss>>ID;
        ss.str("");
        ss.clear();
        ss<<getLineNo();
        ss>>No;
        string str="";
        cout<<"=================================================="<<endl;
        if(getSymbol()==""){str="= AIG(" + ID + "), line "+No;}
        else{str="= AIG(" + ID + ")\""+getSymbol()+"\", line "+No;}
        cout<<setw(49)<<left<<str<<"="<<endl;
        cout<<"=================================================="<<endl;
        ss.str("");
        ss.clear();
      }
      virtual int getID()const{return gateID;}
      virtual pair<int,bool> getPair0() const{return _fanin0;}
      virtual pair<int,bool> getPair1() const{return _fanin1;}
  private:
  pair<int,bool>                 _fanin0;
  pair<int,bool>                 _fanin1;
  int                 gateID;
};

class CirUndefGate:public CirGate
{
  public:
      CirUndefGate(int ID):gateID(ID){setLineNo(0);setType("UNDEF");}
      ~CirUndefGate(){}
      virtual void printGate()const{
        stringstream ss;
        string ID,No;
        ss<<gateID;
        ss>>ID;
        ss.str("");
        ss.clear();
        ss<<getLineNo();
        ss>>No;
        string str="";
        cout<<"=================================================="<<endl;
        if(getSymbol()==""){str="= UNDEF(" + ID + "), line "+"0";}
        else{str="= UNDEF(" + ID + ")\""+getSymbol()+"\", line "+"0";}
        cout<<setw(49)<<left<<str<<"="<<endl;
        cout<<"=================================================="<<endl;
        ss.str("");
        ss.clear();
      }
      virtual int getID()const{return gateID;}
      virtual pair<int,bool> getPair0() const{return pair<int,bool>(0,0);}
      virtual pair<int,bool> getPair1() const{return pair<int,bool>(0,0);}

  private:
  int                 gateID;

};

class CirConstGate:public CirGate
{
  public:
      CirConstGate():gateID(0){setType("const");}
      ~CirConstGate(){}
      virtual void printGate()const{
        string str;
        cout<<"=================================================="<<endl;
        if(getSymbol()==""){str="= CONST(0), line 0";}
        cout<<setw(49)<<left<<str<<"="<<endl;
        cout<<"=================================================="<<endl;
      }
      virtual int getID()const{return gateID;}
      virtual pair<int,bool> getPair0() const{return pair<int,bool>(0,0);}
      virtual pair<int,bool> getPair1() const{return pair<int,bool>(0,0);}

  private:
  int                 gateID;

};
#endif // CIR_GATE_H
