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

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"
#include "myHashMap.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() { _gateList=0; flag=0; ok2FEC=false;}
   ~CirMgr() { delete [] flag; } 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const;

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

   //self-defined hw6
   void resetFlag(){for(size_t i=0;i<_gateNo;++i) flag[i]=0; }
   CirGate* _gateList;//id
   bool* flag;//for constructing _dfsList
   HashMap<SimValueKey,FECGroup> fecGrps;

   

private:
   ofstream           *_simLog;
   int M,I,L,O,A;
   unsigned _gateNo;//total gate no.
   void DFS(int id);
   vector<unsigned> _dfsList;//id,actually includes UNDEF_GATE
   vector<unsigned> _piList;//id
   vector<unsigned> _poList;//id

   //self-defined final 
   void eraseFanout(unsigned,unsigned);
   void changeFanin(unsigned,unsigned,unsigned);
   void simulate(unsigned*);
   bool checkSimulate(unsigned);
   bool ok2FEC;
};

#endif // CIR_MGR_H
