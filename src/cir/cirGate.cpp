/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"
#include "myHashMap.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
	stringstream ss; unsigned id=this-(cirMgr->_gateList);
    ss<<" "<<getTypeStr()<<"("<<id<<")";
    if(_symbolicName.size()) ss<<"\""<<_symbolicName<<"\"";
    ss<<", line "<<_lineNo;
    cout<<"==================================================\n";
    cout<<"="<<setw(48)<<left<<ss.str()<<right<<"="<<endl;
    stringstream sss;
    unsigned row,col;
    cirMgr->fecGrps.check(SimValueKey(SimValue),row,col);
    for(size_t i=0;i<cirMgr->fecGrps[row][col].second.size();++i)
    { 
    	if(cirMgr->fecGrps[row][col].second[i]!=id && (_type==AIG_GATE || _type==CONST_GATE))
    	{
    		if(cirMgr->_gateList[cirMgr->fecGrps[row][col].second[i]].SimValue==(~SimValue))sss<<"!";
    		sss<<cirMgr->fecGrps[row][col].second[i]<<" ";
    	}
    }
    cout<<"= FECs: "<<setw(42)<<left<<sss.str()<<right<<"="<<endl;
    cout<<"= Value: ";
    for(int i=31;i>=0;--i)
    {
    	int j=((SimValue&(1<<i))>>i);
        cout<<j;
        if(i%4==0 && i!=0)cout<<"_";
    }
    cout<<" ="<<endl;
    cout<<"==================================================\n";
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   cirMgr->resetFlag();
   PrintFanin(level,level,0);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   cirMgr->resetFlag();
   PrintFanout(level,level,0);
}

void
CirGate::PrintFanin(int level,int dep,unsigned inver) const
{
    int id = this-cirMgr->_gateList;
    for(int j=0;j<(level-dep)*2;++j) cout<<" ";
    if(inver) cout<<"!";
    cout<<getTypeStr()<<" "<< id;
    if(dep&&cirMgr->flag[id]) {cout<<"  (*)"<<endl; return;}
    if(!dep||cirMgr->flag[id]) return;
    cirMgr->flag[id] = 1;
    for(size_t i=0;i<faninList.size();i++)
    {
        cout<<endl; 
        (cirMgr->_gateList[faninList[i]/2]).PrintFanin(level,dep-1,faninList[i]%2);
    }
}

void
CirGate::PrintFanout(int level,int dep,unsigned inver) const
{
    int id = this-cirMgr->_gateList;
    for(int j=0;j<(level-dep)*2;++j) cout<<" ";
    if(inver) cout<<"!";
    cout<<getTypeStr()<<" "<< id;
    if(dep&&cirMgr->flag[id]) {cout<<"  (*)"<<endl; return;}
    if(!dep||cirMgr->flag[id]) return;
    cirMgr->flag[id] = 1;
    for(size_t i=0;i<fanoutList.size();i++)
    {
        cout<<endl;
        (cirMgr->_gateList[fanoutList[i]/2]).PrintFanout(level,dep-1,fanoutList[i]%2);
    }
}

void 
CirGate::resetGate()
{
	SimValue=0;
	_type=TOT_GATE; 
	_symbolicName=""; 
	_lineNo=0;
	faninList.clear(); 
	fanoutList.clear();
}

/*void
CirGate::simulate(unsigned* patternInput)
{
   unsigned id=this-(cirMgr->_gateList);
   bool change;
   for(unsigned i=0;i<faninList.size();++i)
   {
      if()
   }
}*/



