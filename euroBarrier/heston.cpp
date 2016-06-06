/*----------------------------------------------------------------------------
*
* Author:   Liang Ma (liang-ma@polito.it)
*
*----------------------------------------------------------------------------
*/
#include "heston.h"
#define 	MAX_NUM_RNG 	2
#define 	MAX_SAMPLE      2
#define 	NUM_SHARE	64
#define 	PAR		64


heston::heston(stockData data,volData vol,barrierData bData)
	:data(data),vol(vol),bData(bData)
{
	//vol.print();
}

void heston::sampleSIM(RNG* mt_rng, data_t* call,data_t* put)
{
	const data_t Dt=data.timeT/PAR,
			ratio1=expf(-data.freeRate*data.timeT)*data.strikePrice,
			ratio2=sqrtf(fmaxf(1-vol.correlation*vol.correlation,0)),
			ratio3=Dt*data.freeRate,
			ratio4=vol.kappa*vol.expect*Dt,
			volInit =fmaxf(vol.initValue,0)*Dt,
			upB=bData.upBarrier,
			lowB=bData.lowBarrier;

	data_t fCall=0,fPut=0;
	data_t sCall[MAX_NUM_RNG],sPut[MAX_NUM_RNG];
#pragma HLS ARRAY_PARTITION variable=sCall complete dim=1
#pragma HLS ARRAY_PARTITION variable=sPut complete dim=1

	data_t stockPrice[MAX_NUM_RNG][NUM_SHARE];
#pragma HLS ARRAY_PARTITION variable=stockPrice complete dim=1

	data_t vols[MAX_NUM_RNG][NUM_SHARE],pVols[MAX_NUM_RNG][NUM_SHARE];

#pragma HLS ARRAY_PARTITION variable=vols complete dim=1
#pragma HLS ARRAY_PARTITION variable=pVols complete dim=1

	data_t num1[MAX_NUM_RNG][NUM_SHARE],num2[MAX_NUM_RNG][NUM_SHARE];
#pragma HLS ARRAY_PARTITION variable=num2 complete dim=1
#pragma HLS ARRAY_PARTITION variable=num1 complete dim=1

	data_t bBarrier[MAX_NUM_RNG][NUM_SHARE];
#pragma HLS ARRAY_PARTITION variable=bBarrier complete dim=1

	loop_init:for(int s=0;s<NUM_SHARE;s++)
	{
		for(int i =0;i<MAX_NUM_RNG;i++)
		{
	#pragma HLS UNROLL
			stockPrice[i][s]=data.initPrice;
			vols[i][s]=vol.initValue;
			pVols[i][s]=volInit;
			bBarrier[i][s]=true;
		}
	}
	loop_main:for(int j=0;j<MAX_SAMPLE;j++)
	{
		loop_path:for(int path=0;path<PAR;path++)
		{
			loop_share:for(int s=0;s<NUM_SHARE;s++)
			{
		#pragma HLS PIPELINE
				loop_parallel:for(uint i=0;i<MAX_NUM_RNG;i++)
				{
#pragma HLS UNROLL
					if(!bBarrier[i][s])
						continue;
					mt_rng[i].BOX_MULLER(&num1[i][s],&num2[i][s],pVols[i][s]);

					stockPrice[i][s]*=1+ratio3+num1[i][s]*vol.correlation+num2[i][s]*ratio2;
					if(stockPrice[i][s]<lowB || stockPrice[i][s]>upB)
					{
						bBarrier[i][s]=false;
					}
					vols[i][s]+=ratio4-vol.kappa*pVols[i][s]+vol.variance*num1[i][s];
					pVols[i][s]=fmaxf(vols[i][s],0)*Dt;
				}
			}
		}
		loop_sum:for(int s=0;s<NUM_SHARE;s++)
		{
			loop_sum_R:for(int i =0;i<MAX_NUM_RNG;i++)
			{
#pragma HLS UNROLL
				vols[i][s]=vol.initValue;
				pVols[i][s]=volInit;
				if(bBarrier[i][s])
				{
					if(stockPrice[i][s]>data.strikePrice)
					{
						sCall[i]=stockPrice[i][s]-data.strikePrice;
					}
					else
					{
						sPut[i]+=data.strikePrice-stockPrice[i][s];
					}
				}
				stockPrice[i][s]=data.initPrice;
				bBarrier[i][s]=true;
			}
		}
	}
	loop_final_sum:for(int i =0;i<MAX_NUM_RNG;i++)
	{
#pragma HLS UNROLL
		fCall+=sCall[i];
		fPut+=sPut[i];
	}
	*call= fCall/MAX_NUM_RNG/MAX_SAMPLE/NUM_SHARE;
	*put= fPut/MAX_NUM_RNG/MAX_SAMPLE/NUM_SHARE;
}

void heston::simulation(data_t* pCall, data_t *pPut)
{

	RNG mt_rng[MAX_NUM_RNG];
#pragma HLS ARRAY_PARTITION variable=mt_rng complete dim=1

	uint seeds[MAX_NUM_RNG];
#pragma HLS ARRAY_PARTITION variable=seeds complete dim=1

	loop_seed:for(int i=0;i<MAX_NUM_RNG;i++)
	{
#pragma HLS UNROLL
		seeds[i]=i;
	}
	RNG::init_array(mt_rng,seeds,MAX_NUM_RNG);
	return sampleSIM(mt_rng,pCall,pPut);

}
