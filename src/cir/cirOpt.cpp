/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <algorithm>
#include <iostream>
#include <vector>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"


using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void 
CirMgr::sweep()//update cirprint-floating
{
  vector<unsigned>::iterator it;
  for(unsigned i=0;i<(unsigned)M+1;++i)
  {
    it=find(_dfsList.begin(),_dfsList.end(),i);
    if(it==_dfsList.end() && 
      (_gateList[i].getType()==AIG_GATE || _gateList[i].getType()==UNDEF_GATE))//not found in _dfsList, not PI,not TOT
    {
      cout<<"Sweeping: "<<_gateList[i].getTypeStr()<<"("<<i<<") removed..."<<endl;
      if(_gateList[i].getType()==AIG_GATE)
      {
        A--;
        unsigned fanin1=_gateList[i].faninList[0]/2;
        unsigned fanin2=_gateList[i].faninList[1]/2;
        eraseFanout(i,fanin1);
        eraseFanout(i,fanin2);
      }
      _gateList[i].resetGate();
    }
  }
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
	for(unsigned i=0;i<_dfsList.size();++i)
	{
		unsigned ID=_dfsList[i];
		unsigned faninID,faninINV;
		if(_gateList[ID].getType()!=AIG_GATE)continue;
		
	  //same input
		if(_gateList[ID].faninList[0]==_gateList[ID].faninList[1])
		{
			faninID=_gateList[ID].faninList[0]/2;
      faninINV=_gateList[ID].faninList[0]%2;
      cout<<"Simplifying: "<<faninID<<" merging ";
      if(faninINV)cout<<"!";
      cout<<ID<<"..."<< endl;
      changeFanin(ID,faninID,faninINV);
      eraseFanout(ID,faninID);
      eraseFanout(ID,faninID);
      _gateList[ID].resetGate();
      A--;
		}
	  
    //inverse input
	  else if((_gateList[ID].faninList[0]/2 == _gateList[ID].faninList[1]/2) 
	    	 && (_gateList[ID].faninList[0]!= _gateList[ID].faninList[1]))
		{
			faninID=_gateList[ID].faninList[0]/2;
      cout<<"Simplifying: 0 merging "<<ID<<"..."<<endl;
      changeFanin(ID,0,0);
      eraseFanout(ID,faninID);
      eraseFanout(ID,faninID);
      _gateList[ID].resetGate();
      A--;
		}

		//const input situation
		else
		{
			for(unsigned j=0;j<2;++j)
			{
				if(_gateList[ID].faninList[j]<=1)//0 or 1
				{
	        unsigned Const=_gateList[ID].faninList[j];
	        faninID=_gateList[ID].faninList[!j]/2;
	        faninINV=_gateList[ID].faninList[!j]%2;
	        if(!Const)
          {
            cout<<"Simplifying: 0 merging "<<ID<<"..."<<endl;
            changeFanin(ID,0,0);
          }
          else                    
          {
            cout<<"Simplifying: "<<faninID<<" merging ";
            if(faninINV)cout<<"!";
            cout<<ID<<"..."<< endl;
            changeFanin(ID,faninID,faninINV);
          }
	        eraseFanout(ID,0);
	        eraseFanout(ID,faninID);
	        _gateList[ID].resetGate();
	        A--;
	      }
	    }
	  }	
	}
	//_dfslist
	resetFlag();
  _dfsList.clear();
  for(size_t i=0;i<_poList.size();++i) {DFS(_poList[i]);}
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void
CirMgr::eraseFanout(unsigned id,unsigned fanin)
{
  for(unsigned i=0;i<_gateList[fanin].fanoutList.size();++i)
  {
    if(_gateList[fanin].fanoutList[i]/2==id)
    { _gateList[fanin].fanoutList.erase(_gateList[fanin].fanoutList.begin()+i);return; }
  }
}

void
CirMgr::changeFanin(unsigned id,unsigned newid,unsigned faninINV)
{
  for(size_t i=0;i<_gateList[id].fanoutList.size();++i)
  {
    unsigned fanoutID=_gateList[id].fanoutList[i]/2;
    unsigned fanoutINV=_gateList[id].fanoutList[i]%2;
    _gateList[newid].pushFanout(fanoutID*2+(faninINV+fanoutINV)%2);
    for(size_t j=0;j<_gateList[fanoutID].faninList.size();++j)
    {
      if(_gateList[fanoutID].faninList[j]/2==id)
         _gateList[fanoutID].faninList[j]=newid*2+(faninINV+fanoutINV)%2;
    }
  }
}

