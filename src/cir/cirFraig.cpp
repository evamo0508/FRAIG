/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
  HashMap<strashKey,CirGateV> hash(getHashSize(_dfsList.size()));
  for(unsigned i=0;i<_dfsList.size();++i)
  {
  	unsigned ID=_dfsList[i];
	  if(_gateList[ID].getType()!=AIG_GATE)continue;
	  strashKey k(_gateList[ID].faninList[0],_gateList[ID].faninList[1]);
	  CirGateV gate(ID);
	  unsigned  mergeGate;//ID
	  if (hash.check(k,mergeGate) == true)// mergeGate is set when found
    {
    	cout<<mergeGate<<" merging "<<ID<<"..."<<endl;
    	changeFanin(ID,mergeGate,0);
      eraseFanout(ID,_gateList[ID].faninList[0]/2);
      eraseFanout(ID,_gateList[ID].faninList[1]/2);
      _gateList[ID].resetGate();
	    A--;
    }
    else hash.forceInsert(k,gate);
  }
  //_dfslist
  resetFlag();
  _dfsList.clear();
  for(size_t i=0;i<_poList.size();++i) {DFS(_poList[i]);}
}

void
CirMgr::fraig()
{
  if(!ok2FEC) {cout<<"sim before fraig..."<<endl; return;}

  ok2FEC=false;
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
