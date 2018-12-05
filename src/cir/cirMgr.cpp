/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

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
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
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

bool
getTokens(ifstream& ifs,vector<string>& tokens)
{
  tokens.clear();
  string line="";
  getline(ifs,line);
  string token="";
  size_t pos=myStrGetTok(line,token);
  if(!line.size())return false;
  while(token.size())
  {
    tokens.push_back(token);
    pos=myStrGetTok(line,token,pos);
  }
  return true;
}

CirGate* 
CirMgr::getGate(unsigned gid) const 
{ 
  if(gid>_gateNo)return 0; 
  if(_gateList[gid].getType() == TOT_GATE) return 0;
  return (_gateList+gid);
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   ifstream ifs(fileName.c_str(),std::ifstream::in);
   if(!ifs.is_open()) return false;
   vector<string> tokens;

   //header
   getTokens(ifs,tokens);
   myStr2Int(tokens[1],M);
   myStr2Int(tokens[2],I);
   myStr2Int(tokens[3],L);
   myStr2Int(tokens[4],O);
   myStr2Int(tokens[5],A);
   _gateNo=M+O+1;//1:const 0
   _gateList = new CirGate[_gateNo];
   _gateList[0].setType(CONST_GATE);
   flag = new bool[_gateNo];
   resetFlag();
   lineNo=1;
   fecGrps.reset();
   //++lineNo;

   //input
   int lit;
   for(int i=0;i<I;++i){
    getTokens(ifs,tokens);
    myStr2Int(tokens[0],lit);
    _gateList[lit/2].setType(PI_GATE);
    _gateList[lit/2].setLine(++lineNo);
    _piList.push_back(lit/2); 
   }

   //output
   int id;
   for(int i=0;i<O;++i){
    id=M+i+1;
    getTokens(ifs,tokens);
    myStr2Int(tokens[0],lit);
    _gateList[id].setType(PO_GATE);
    _gateList[id].setLine(++lineNo);
    _gateList[id].pushFanin(unsigned(lit));
    if(_gateList[lit/2].getTypeStr()=="TOT")_gateList[lit/2].setType(UNDEF_GATE);
    _gateList[lit/2].pushFanout(unsigned(id*2+lit%2));
    _poList.push_back(id);
   }

   //AIG
   int fanout1,fanout2;
   for(int i=0;i<A;++i){
    getTokens(ifs,tokens);
    myStr2Int(tokens[0],lit);
    myStr2Int(tokens[1],fanout1);
    myStr2Int(tokens[2],fanout2);
    _gateList[lit/2].setType(AIG_GATE);
    _gateList[lit/2].setLine(++lineNo);
    _gateList[lit/2].pushFanin(unsigned(fanout1));
    _gateList[lit/2].pushFanin(unsigned(fanout2));
    if(_gateList[fanout1/2].getTypeStr()=="TOT")_gateList[fanout1/2].setType(UNDEF_GATE);
    if(_gateList[fanout2/2].getTypeStr()=="TOT")_gateList[fanout2/2].setType(UNDEF_GATE);
    _gateList[fanout1/2].pushFanout(unsigned(lit+fanout1%2));
    _gateList[fanout2/2].pushFanout(unsigned(lit+fanout2%2));
   }

   //symbol
   while(ifs>>buf)//stops at whitespace char
   {
      if(buf[0]=='c') break;
      else if(buf[0]=='i')
      {
        myStr2Int(string(buf+1),id);//here id is actually the position that input is defined 
        string name;
        getline(ifs,name);
        name.erase(0,1);//don't want first whitespace only
        _gateList[_piList[id]].setSymbolicName(name);
      }
      else if(buf[0]=='o')
      {
        myStr2Int(string(buf+1),id);
        string name;
        getline(ifs,name);
        name.erase(0,1);
        _gateList[_poList[id]].setSymbolicName(name);
      }
   }
   
   //_dfslist
   _dfsList.clear();
   for(size_t i=0;i<_poList.size();++i) {DFS(_poList[i]);}
   return true;
}

void
CirMgr::DFS (int id)
{
  flag[id] = 1;
  for(size_t i=0;i<_gateList[id].faninList.size();++i)
  {
    unsigned tmpId = (_gateList[id].faninList[i])/2;
    if(!flag[tmpId]) DFS(tmpId);
  }
  _dfsList.push_back(id);
}

// Member functions about circuit optimization

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
   cout<<"\nCircuit Statistics\n==================\n";
   cout<<"  PI"<<setw(12)<<I<<endl;
   cout<<"  PO"<<setw(12)<<O<<endl;
   cout<<"  AIG"<<setw(11)<<A<<endl;
   cout<<"------------------"<<endl;
   cout<<"  Total"<<setw(9)<<I+O+A<<endl;
}

void
CirMgr::printNetlist() const
{
/*
   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _dfsList[i]->printGate();
   }
*/
  int count=0;
  for(size_t i=0;i<_dfsList.size();++i)
  {
    if(_gateList[_dfsList[i]].getType() == UNDEF_GATE) continue;
    cout<<"["<<count++<<"] ";
    unsigned id = _dfsList[i];
    cout<<_gateList[id].getTypeStr()<<" "<<id;
    for(size_t j=0;j<_gateList[id].faninList.size();++j )
    {
      cout<<" ";
      unsigned tmpLit=_gateList[id].faninList[j];
      if(_gateList[tmpLit/2].getType()==UNDEF_GATE) cout<<"*";
      if(tmpLit%2) cout<<"!";
      cout<<tmpLit/2;
    }
    if(_gateList[_dfsList[i]].getName().size())
      cout<<" ("<<_gateList[_dfsList[i]].getName()<<")";
    cout<<endl;
  }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(size_t i=0;i<_piList.size();++i) cout<<" "<<_piList[i];
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(size_t i=0;i<_poList.size();++i) cout<<" "<<_poList[i];
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
  int first=0;
  for(size_t i=0;i<_gateNo;++i)
  {
      if(_gateList[i].getType()==PO_GATE ||_gateList[i].getType()==AIG_GATE )
      {
          for(size_t j=0;j<_gateList[i].faninList.size();++j)
          if(_gateList[_gateList[i].faninList[j]/2].getType() == UNDEF_GATE)
          {
            ++first;
            if(first==1)cout<<"Gates with floating fanin(s):";
            cout<<" "<<i; break;
          }
      }
  }
  if(first)cout<<endl;
  first=0;
  for(size_t i=0;i<_gateNo;++i)
  {
      if(_gateList[i].getType()==PI_GATE ||_gateList[i].getType()==AIG_GATE )
          if(_gateList[i].fanoutList.empty()) 
          {
            ++first;
            if(first==1)cout<<"Gates defined but not used :";
            cout<<" "<<i;
          }
  }
  if(first)cout<<endl;
}
void
CirMgr::printFECPairs() const
{
   HashMap<SimValueKey,FECGroup>::iterator li=fecGrps.begin();
   int count=0;
   li++;
   while(li!=fecGrps.end())
   {
      cout<<"["<<count<<"] ";
      unsigned k=(*li).first.getSimValue();
      for(size_t i=0;i<(*li).second.size();++i)
      {
        if(i!=0 && _gateList[(*li).second[i]].SimValue==(~k))cout<<"!";
        cout<<(*li).second[i]<<" ";
      }
      cout<<endl;
      count++;
      li++;
   }
}

void
CirMgr::writeAag(ostream& outfile) const
{
  int _A=0;
  for(size_t i=0;i<_dfsList.size();++i)
  {if(_gateList[_dfsList[i]].getType()==AIG_GATE) ++_A;}
  outfile<<"aag "<<M<<" "<<I<<" "<<L<<" "<<O<<" "<<_A<<endl;
  for(size_t i=0;i<_piList.size();++i)
  outfile<<_piList[i]*2<<endl;
  for(size_t i=0;i<_poList.size();++i)
  outfile<<_gateList[_poList[i]].faninList[0]<<endl;
  for(size_t i=0;i<_dfsList.size();++i)
  if(_gateList[_dfsList[i]].getType()==AIG_GATE)
  outfile<<_dfsList[i]*2<<" "<<_gateList[_dfsList[i]].faninList[0]
         <<" "<<_gateList[_dfsList[i]].faninList[1]<<endl;
  for(size_t i=0;i<_piList.size();++i)
  if(_gateList[_piList[i]].getName().size()) 
    outfile<<"i"<<i<<" "<<_gateList[_piList[i]].getName()<<endl;
  for(size_t i=0;i<_poList.size();++i)
  if(_gateList[_poList[i]].getName().size()) 
    outfile<<"o"<<i<<" "<<_gateList[_poList[i]].getName()<<endl;
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
}

