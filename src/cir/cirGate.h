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
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
   CirGate():SimValue(0),_type(TOT_GATE),_symbolicName(""),_lineNo(0){}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const 
   { 
    switch(_type)
    {
      case 0: return "UNDEF";break;
      case 1: return "PI";break;
      case 2: return "PO";break;
      case 3: return "AIG";break;
      case 4: return "CONST";break;
      default: return "TOT";
    }
   }
   unsigned getLineNo() const { return _lineNo; }
   /*virtual*/ bool isAig() const { return false; }

   // Printing functions
   /*virtual*/ void printGate() const {}
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
   void PrintFanin(int level,int dep,unsigned inver) const;
   void PrintFanout(int level,int dep,unsigned inver) const;

   //self-add function
   void setSymbolicName(const string& name) {_symbolicName=name;}
   void pushFanin (const unsigned& lit){faninList.push_back(lit);}
   void pushFanout(const unsigned& lit){fanoutList.push_back(lit);}
   void setType(const GateType& t){_type=t;}
   void setLine(const unsigned& line) {_lineNo = line;}
   GateType getType() const {return _type;}
   string getName() const {return _symbolicName;}
   void resetGate();//for sweeping
   //bool simulate(unsigned* patternInput);
   //void setSimValue(unsigned s){SimValue=s;}
   //unsigned getSimValue const() {return SimValue;}

   //member
   vector<unsigned> faninList;//lit
   vector<unsigned> fanoutList;//lit
   unsigned SimValue; 
   
private:
  GateType _type;
  string _symbolicName;
  unsigned _lineNo;
  
protected:
};

#endif // CIR_GATE_H
