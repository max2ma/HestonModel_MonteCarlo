/*----------------------------------------------------------------------------
*
* Author:   Liang Ma (liang-ma@polito.it)
*
*----------------------------------------------------------------------------
*/
#include "heston.h"
#define 	MAX_NUM_RNG 	4
#define 	MAX_SAMPLE 		1
#define 	NUM_SHARE		64
#define 	PAR				64


#ifdef __CLIANG__
#include<iostream>
using namespace std;
#endif

heston::heston(stockData data,volData vol):data(data),vol(vol)
{
	//vol.print();
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

void heston::sampleSIM(RNG* mt_rng, data_t* call,data_t* put)
{
	const data_t Dt=data.timeT/PAR,
			ratio1=expf(-data.freeRate*data.timeT)*data.strikePrice,
			ratio2=sqrtf(fmaxf(1-vol.correlation*vol.correlation,0)),
			ratio3=Dt*data.freeRate,ratio4=vol.kappa*vol.expect*Dt,
			logPrice = logf(data.initPrice/data.strikePrice),
			volInit = fmaxf(vol.initValue,0)*Dt;

	const int sPath=MAX_NUM_RNG*NUM_SHARE;
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

	loop_init:for(int s=0;s<NUM_SHARE;s++)
	{
		for(int i =0;i<MAX_NUM_RNG;i++)
		{
	#pragma HLS UNROLL
			stockPrice[i][s]=logPrice;
			vols[i][s]=vol.initValue;
			pVols[i][s]=volInit;
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
					mt_rng[i].BOX_MULLER(&num1[i][s],&num2[i][s],pVols[i][s]);
					vols[i][s]+=ratio4-vol.kappa*pVols[i][s]+vol.variance*num1[i][s];

					stockPrice[i][s]+=ratio3-pVols[i][s]*0.5f
							+(num1[i][s]*vol.correlation+num2[i][s]*ratio2);

					pVols[i][s]=fmaxf(vols[i][s],0)*Dt;
				}
			}

		}

		loop_sum:for(int s=0;s<NUM_SHARE;s++)
		{
			loop_sum_r:for(int i =0;i<MAX_NUM_RNG;i++)
			{
	#pragma HLS UNROLL
				//cout<<"sim="<<j<<" price="<<expf(pCall[i])*data.strikePrice<<endl;
				if(stockPrice[i][s]>0)
				{
					sCall[i]+=expf(stockPrice[i][s])-1.0f;
				}
				else
				{
					sPut[i]+=1.0f-expf(stockPrice[i][s]);
				}
				stockPrice[i][s]=logPrice;
				vols[i][s]=vol.initValue;
				pVols[i][s]=volInit;
			}
		}
	}
	loop_final_sum:for(int i =0;i<MAX_NUM_RNG;i++)
	{
#pragma HLS UNROLL
		fCall+=sCall[i];
		fPut+=sPut[i];
	}
	*call= ratio1*fCall/MAX_NUM_RNG/MAX_SAMPLE/NUM_SHARE;
	*put= ratio1*fPut/MAX_NUM_RNG/MAX_SAMPLE/NUM_SHARE;
}

