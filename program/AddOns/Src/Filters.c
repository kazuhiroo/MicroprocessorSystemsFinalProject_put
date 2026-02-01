/*
  * @file     Filters.c
 * @author  a.katowski, https://github.com/kazuhiroo
 * @author  o.jozwik, https://github.com/oskarjozwik
 * @author  i.kolas, https://github.com/ililich
  * @date     Jan 17, 2026
  * @brief    Implemented filters for the signals in the loop
  *
*/

#include "Filters.h"


static float input_buffer[AVG_FILT_SQ];

/*
 * @brief   : reset average filter samples.
 *
 * */
void AvgFilter_Reset(void){
    for(int i=0;i<AVG_FILT_SQ;i++){
        input_buffer[i] = 0.0f;
    }
}

/*
 * @brief   : moving average filter defined by the samples quantity.
 *
 * */
float AvgFilter(float input){
	//static float input_buffer[AVG_FILT_SQ] = {0};
	float output = 0.0f;
	float sum = 0.0f;

	for(int i = 0; i < AVG_FILT_SQ - 1; i++){
		input_buffer[i] = input_buffer[i+1];
	}

	input_buffer[AVG_FILT_SQ-1] = input;

	for(int i = 0; i < AVG_FILT_SQ; i++){
		sum += input_buffer[i];
	}

	output = sum / AVG_FILT_SQ;

	return output;
}
