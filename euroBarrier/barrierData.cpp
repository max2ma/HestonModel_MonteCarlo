/*----------------------------------------------------------------------------
*
* Author:   Liang Ma (liang-ma@polito.it)
*
*----------------------------------------------------------------------------
*/

#include "barrierData.h"
#ifdef __CLIANG__
#include <iostream>
using namespace std;
#endif
barrierData::barrierData(data_t upBarrier, data_t lowBarrier)
{
	if(upBarrier<lowBarrier)
	{
		this->upBarrier=0;
		this->lowBarrier=0;
	}
	else
	{
		this->upBarrier=upBarrier;
		this->lowBarrier=lowBarrier;
	}
}

barrierData::barrierData(const barrierData& data)
{
	this->upBarrier=data.upBarrier;
	this->lowBarrier=data.lowBarrier;
}

bool barrierData::checkRange(data_t stockPrice)const
{
	return (stockPrice>=lowBarrier)&&(stockPrice<=upBarrier);
}

#ifdef __CLIANG__
void barrierData::print()const
{
	cout<<"lowBarrier="<<lowBarrier<<' '
			<<"upBarrier="<<upBarrier<<endl;
}
#endif
