/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "myHashMap.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{

	ok2FEC=true;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
    //initial
    fecGrps.init(getHashSize(_dfsList.size()));
    FECGroup FECGrp;
    for(unsigned i=0;i<_dfsList.size();++i)
    {
    	unsigned ID=_dfsList[i];
	    if(_gateList[ID].getType()!=AIG_GATE && _gateList[ID].getType()!=CONST_GATE)continue;
	    FECGrp.pushID(ID);
    }
    if(!fecGrps.insert(SimValueKey(0),FECGrp)){cout<<"FECGrp initialize failed";return;}

    //read file
    //ifstream ifs(patternFile.c_str(),std::ifstream::in);
    //if(!ifs.is_open()) {cout<<"file reading failed"; return;}
    string str;
    unsigned patternNO=0;
    unsigned* patternInput=0;
    while(patternFile>>str)
    {

    	if(patternNO%32==0)
    	{
    		if(patternNO!=0)delete [] patternInput; 
    		patternInput=new unsigned[str.size()];
            for(size_t i=0;i<str.size();++i)patternInput[i]=0;
            //patternNO=0;
    	}
    	
    	if(str.size()!=_piList.size()) 
    	{
    		cout<<"Error: Pattern("<<str<<") length("<<str.size()
    		    <<") does not match the number of inputs("<<_piList.size()<<") in a circuit!!";
            //delete [] patternInput; 
    		return;
        }
        
        for(size_t i=0;i<str.size();++i)
        {
        	if(str[i]!='0' && str[i]!='1')
        	{
        		cout<<"Error: Pattern("<<str<<") contains a non-0/1 character(‘"<<str[i]<<"’).";
        	    //delete [] patternInput; 
        	    return;
        	}
        	patternInput[i]^=(unsigned)((str[i]-48)<<(patternNO%32));
        }

        if(patternNO%32==31)
        {
            //simulation
            simulate(patternInput);

            HashMap<SimValueKey,FECGroup>::iterator li=fecGrps.begin();
            //HashMap<SimValueKey,FECGroup>::iterator *Li=new HashMap<SimValueKey,FECGroup>::iterator[fecGrps.size()];
            while(li!=fecGrps.end()) {(*li).first.setFlag(1);li++;}
            li=fecGrps.begin();
            while(li!=fecGrps.end())
            {
                HashMap<SimValueKey,FECGroup> newFecGrps(getHashSize(_dfsList.size()));
                for(size_t j=0;j<(*li).second.size();++j)
                {
       	            SimValueKey s(_gateList[(*li).second[j]].SimValue);
       	            unsigned row,col;
       	            if(newFecGrps.check(s,row,col)) newFecGrps[row][col].second.pushID((*li).second[j]);
       	            else
       	            {
                        FECGroup f;
                        f.pushID((*li).second[j]);
       	        	    newFecGrps.forceInsert(s,f);
       	            } 
                }

                //collectValidFecGrp(newFecGrps,fecGrp,fecGrps);
                fecGrps.remove((*li).first);
                HashMap<SimValueKey,FECGroup>::iterator newLi=newFecGrps.begin();
                while(newLi!=newFecGrps.end()) {fecGrps.forceInsert((*newLi).first,(*newLi).second);++newLi;}
                newFecGrps.reset();
                while((*li).first.getFlag()!=1)++li;
            }
            //delete [] Li;

        }
        patternNO++;       
    }

    cout<<patternNO<<" patterns simulated.";
	ok2FEC=true;
	//delete [] patternInput;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void
CirMgr::simulate(unsigned* patternInput)
{
    for(size_t i=0;i<_piList.size();++i)_gateList[_piList[i]].SimValue=patternInput[i];
    for(size_t i=0;i<_poList.size();++i)checkSimulate(_poList[i]);
}

bool
CirMgr::checkSimulate(unsigned id)
{
	bool change=false;
    for(unsigned i=0;i<_gateList[id].faninList.size();++i)
    {
      if(checkSimulate(_gateList[id].faninList[i]/2))change=true;
    }
    if(_gateList[id].getType()==PI_GATE) return true;
    if(!change)return false;
    unsigned oldSimValue=_gateList[id].SimValue;
    unsigned newSimValue;
    if(_gateList[id].getType()==PO_GATE) 
    {
        unsigned fanin=_gateList[id].faninList[0]/2;
    	newSimValue=((_gateList[id].faninList[0]%2)?(~_gateList[fanin].SimValue):_gateList[fanin].SimValue);
    }
    else if(_gateList[id].getType()==AIG_GATE) 
    {
        unsigned fanin1=_gateList[id].faninList[0]/2;
        unsigned fanin2=_gateList[id].faninList[1]/2;
    	newSimValue=((_gateList[id].faninList[0]%2)?(~_gateList[fanin1].SimValue):_gateList[fanin1].SimValue)
                   &((_gateList[id].faninList[1]%2)?(~_gateList[fanin2].SimValue):_gateList[fanin2].SimValue);  
    }
    _gateList[id].SimValue=newSimValue;
    return (oldSimValue!=newSimValue);
}