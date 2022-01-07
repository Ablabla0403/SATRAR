#include <iostream>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include <algorithm>
#include "sat.h"
#include <time.h>

using namespace std;

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

// [0]  PI  1 (c)
// [1]  PI  2 (b)
// [2]  PI  3 (d)
// [3]  PI  4 (e)
// [4]  PI  5 (a)
// [5]  PI  6 (f)
// [6]  AIG 7   2   3
// [7]  AIG 8   4  !1
// [8]  AIG 9   5   2
// [9]  AIG 10  1   7
// [10] AIG 11 !7  !8
// [11] AIG 12 !8  !3
// [12] AIG 13 !12  9
// [13] AIG 14 !10 !13
// [14] AIG 15 !14  6
// [15] PO  16 !11
// [16] PO  17  15 

vector<Gate *> gates;

void
initCircuit()
{
   // Init gates
   for(int i=0; i<17; ++i) {
      gates.push_back(new Gate(i));
   }

   // POs are not needed in this demo example
}

void
genProofModel(SatSolver& s)
{
   // Allocate and record variables; No Var ID for POs
   for (size_t i = 0, n = gates.size(); i < n; ++i) {
      Var v = s.newVar();
      gates[i]->setVar(v);
   }

   // [6]  AIG 7   2   3
   s.addAigCNF(gates[6]->getVar(), gates[1]->getVar(), false,
               gates[2]->getVar(), false);
   // [7]  AIG 8   4  !1
   s.addAigCNF(gates[7]->getVar(), gates[3]->getVar(), false,
               gates[0]->getVar(), true);
   // [8]  AIG 9   5   2
   s.addAigCNF(gates[8]->getVar(), gates[4]->getVar(), false,
               gates[1]->getVar(), false);
   // [9]  AIG 10  1   7
   s.addAigCNF(gates[9]->getVar(), gates[0]->getVar(), false,
               gates[6]->getVar(), false);
   // [10] AIG 11 !7  !8
   s.addAigCNF(gates[10]->getVar(), gates[6]->getVar(), true,
               gates[7]->getVar(), true);
   // [11] AIG 12 !8  !3
   s.addAigCNF(gates[11]->getVar(), gates[7]->getVar(), true,
               gates[2]->getVar(), true);
   // [12] AIG 13 !12  9
   s.addAigCNF(gates[12]->getVar(), gates[11]->getVar(), true,
               gates[8]->getVar(), false);
   // [13] AIG 14 !10 !13
   s.addAigCNF(gates[13]->getVar(), gates[9]->getVar(), true,
               gates[12]->getVar(), true);
   // [14] AIG 15 !14  6
   s.addAigCNF(gates[14]->getVar(), gates[13]->getVar(), true,
               gates[5]->getVar(), false);
   
}

void reportResult(const SatSolver& solver, bool result, int num, int color, int steps)
{
   solver.printStats();
   cout << (result? "SAT" : "UNSAT") << endl;
}


int main()
{
   // initCircuit();

   SatSolver solver;
   solver.initialize();

   bool result;

   Var newV = solver.newVar();
   solver.assumeRelease();  // Clear assumptions
   solver.assumeProperty(newV, true);  // k = 1
   result = solver.assumpSolve();
}
