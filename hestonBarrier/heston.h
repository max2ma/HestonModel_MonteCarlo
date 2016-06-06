/*----------------------------------------------------------------------------
*
* Author:   Liang Ma (liang-ma@polito.it)
*
*----------------------------------------------------------------------------
*/
#ifndef __HESTON_H__
#define __HESTON_H__

#include "barrierData.h"
#include "../common/stockData.h"
#include "../common/volatilityData.h"
#include "../common/RNG.h"

class heston
{
	const stockData data;
	const volData vol;
	const barrierData bData;
public:
	heston(stockData,volData,barrierData);
	void simulation(data_t*,data_t*);
	void sampleSIM(RNG *,data_t*,data_t*);
};

#endif
