/*----------------------------------------------------------------------------
*
* Author:   Liang Ma (liang-ma@polito.it)
*
*----------------------------------------------------------------------------
*/
#include "heston.h"

const int heston::NUM_RNGS=2;
const int heston::NUM_SIMS=512;
const int heston::NUM_SIMGROUPS=32;
const int heston::NUM_STEPS=128;

heston::heston(stockData data,volData vol,barrierData bData)
	:data(data),vol(vol),bData(bData)
{
}
void heston::simulation(data_t* pCall, data_t *pPut)
{

	RNG mt_rng[NUM_RNGS];
#pragma HLS ARRAY_PARTITION variable=mt_rng complete dim=1

	uint seeds[NUM_RNGS];
#pragma HLS ARRAY_PARTITION variable=seeds complete dim=1

	loop_seed:for(int i=0;i<NUM_RNGS;i++)
	{
#pragma HLS UNROLL
		seeds[i]=i;
	}
	RNG::init_array(mt_rng,seeds,NUM_RNGS);
	return sampleSIM(mt_rng,pCall,pPut);

}
void heston::sampleSIM(RNG* mt_rng, data_t* call,data_t* put)
{
	const data_t Dt=data.timeT/NUM_STEPS,
			ratio1=expf(-data.freeRate*data.timeT)*data.strikePrice,
			ratio2=sqrtf(fmaxf(1-vol.correlation*vol.correlation,0)),
			ratio3=Dt*data.freeRate,
			ratio4=vol.kappa*vol.expect*Dt,
			volInit =fmaxf(vol.initValue,0)*Dt,
			upB=bData.upBarrier,
			lowB=bData.lowBarrier;

	data_t fCall=0,fPut=0;
	data_t sCall[NUM_RNGS],sPut[NUM_RNGS];
#pragma HLS ARRAY_PARTITION variable=sCall complete dim=1
#pragma HLS ARRAY_PARTITION variable=sPut complete dim=1

	data_t stockPrice[NUM_RNGS][NUM_SIMGROUPS];
#pragma HLS ARRAY_PARTITION variable=stockPrice complete dim=1

	data_t vols[NUM_RNGS][NUM_SIMGROUPS],pVols[NUM_RNGS][NUM_SIMGROUPS];

#pragma HLS ARRAY_PARTITION variable=vols complete dim=1
#pragma HLS ARRAY_PARTITION variable=pVols complete dim=1

	data_t num1[NUM_RNGS][NUM_SIMGROUPS],num2[NUM_RNGS][NUM_SIMGROUPS];
#pragma HLS ARRAY_PARTITION variable=num2 complete dim=1
#pragma HLS ARRAY_PARTITION variable=num1 complete dim=1

	data_t bBarrier[NUM_RNGS][NUM_SIMGROUPS];
#pragma HLS ARRAY_PARTITION variable=bBarrier complete dim=1

	loop_init:for(int s=0;s<NUM_SIMGROUPS;s++)
	{
		for(int i =0;i<NUM_RNGS;i++)
		{
	#pragma HLS UNROLL
			stockPrice[i][s]=data.initPrice;
			vols[i][s]=vol.initValue;
			pVols[i][s]=volInit;
			bBarrier[i][s]=true;
		}
	}
	loop_main:for(int j=0;j<NUM_SIMS;j++)
	{
		loop_path:for(int path=0;path<NUM_STEPS;path++)
		{
			loop_share:for(int s=0;s<NUM_SIMGROUPS;s++)
			{
		#pragma HLS PIPELINE
				loop_parallel:for(uint i=0;i<NUM_RNGS;i++)
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
		loop_sum:for(int s=0;s<NUM_SIMGROUPS;s++)
		{
			loop_sum_R:for(int i =0;i<NUM_RNGS;i++)
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
	loop_final_sum:for(int i =0;i<NUM_RNGS;i++)
	{
#pragma HLS UNROLL
		fCall+=sCall[i];
		fPut+=sPut[i];
	}
	*call= ratio1*fCall/NUM_RNGS/NUM_SIMS/NUM_SIMGROUPS;
	*put= ratio1*fPut/NUM_RNGS/NUM_SIMS/NUM_SIMGROUPS;
}


