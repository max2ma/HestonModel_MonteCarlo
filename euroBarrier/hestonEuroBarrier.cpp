/*----------------------------------------------------------------------------
*
* Author:   Liang Ma (liang-ma@polito.it)
*
*----------------------------------------------------------------------------
*/

#include "heston.h"
void hestonEuroBarrier(data_t *pCall, data_t *pPut,   // call price and put price
		data_t expect,				// theta
		data_t kappa,				// kappa
		data_t variance,			// xi
		data_t correlation,			// rho
		data_t timeT,				// time period of options
		data_t freeRate,			// interest rate of the riskless asset
		data_t volatility,			// volatility of the risky asset
		data_t initPrice,			// stock price at time 0
		data_t strikePrice,			// strike price
		data_t upB,				// up barrier
		data_t lowB)				// low barrier
{
#pragma HLS INTERFACE m_axi port=pCall bundle=gmem
#pragma HLS INTERFACE s_axilite port=pCall bundle=control
#pragma HLS INTERFACE m_axi port=pPut bundle=gmem
#pragma HLS INTERFACE s_axilite port=pPut bundle=control
#pragma HLS INTERFACE s_axilite port=correlation bundle=gmem
#pragma HLS INTERFACE s_axilite port=correlation bundle=control
#pragma HLS INTERFACE s_axilite port=variance bundle=gmem
#pragma HLS INTERFACE s_axilite port=variance bundle=control
#pragma HLS INTERFACE s_axilite port=kappa bundle=gmem
#pragma HLS INTERFACE s_axilite port=kappa bundle=control
#pragma HLS INTERFACE s_axilite port=expect bundle=gmem
#pragma HLS INTERFACE s_axilite port=expect bundle=control
#pragma HLS INTERFACE s_axilite port=timeT bundle=gmem
#pragma HLS INTERFACE s_axilite port=timeT bundle=control
#pragma HLS INTERFACE s_axilite port=freeRate bundle=gmem
#pragma HLS INTERFACE s_axilite port=freeRate bundle=control
#pragma HLS INTERFACE s_axilite port=volatility bundle=gmem
#pragma HLS INTERFACE s_axilite port=volatility bundle=control
#pragma HLS INTERFACE s_axilite port=initPrice bundle=gmem
#pragma HLS INTERFACE s_axilite port=initPrice bundle=control
#pragma HLS INTERFACE s_axilite port=strikePrice bundle=gmem
#pragma HLS INTERFACE s_axilite port=strikePrice bundle=control
#pragma HLS INTERFACE s_axilite port=lowB bundle=gmem
#pragma HLS INTERFACE s_axilite port=lowB bundle=control
#pragma HLS INTERFACE s_axilite port=upB bundle=gmem
#pragma HLS INTERFACE s_axilite port=upB bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

	volData vol(expect,kappa,variance,volatility,correlation);
	stockData sd(timeT,freeRate,volatility,initPrice,strikePrice);
	barrierData bD(upB,lowB);
	heston bs(sd,vol,bD);
	data_t call,put;
	bs.simulation(&call,&put);
	
	*pCall=call;
	*pPut=put;
	return;
}
/*
#include "hestonEuroBarrier.h"
#include "heston.h"



void hestonEuroBarrier(data_t acc,data_t *pCall,data_t *pPut,
		stockData *data,volData *vol,barrierData *bD)
{
#pragma HLS INTERFACE s_axilite port=acc bundle=control
#pragma HLS INTERFACE s_axilite port=acc  bundle=gmem
#pragma HLS INTERFACE s_axilite port=pCall bundle=gmem
#pragma HLS INTERFACE s_axilite port=pCall bundle=control
#pragma HLS INTERFACE s_axilite port=pPut bundle=gmem
#pragma HLS INTERFACE s_axilite port=pPut bundle=control
#pragma HLS INTERFACE s_axilite port=data bundle=control
#pragma HLS INTERFACE s_axilite port=data  bundle=gmem
#pragma HLS INTERFACE s_axilite port=vol bundle=control
#pragma HLS INTERFACE s_axilite port=vol  bundle=gmem
#pragma HLS INTERFACE s_axilite port=bD bundle=control
#pragma HLS INTERFACE s_axilite port=bD  bundle=gmem
#pragma HLS INTERFACE s_axilite port=return bundle=control

	heston bs(*data,*vol,*bD,acc);
	data_t call,put;
	bs.simulation(&call,&put);
	*pCall=call;
	*pPut=put;
	return;
}
*/
