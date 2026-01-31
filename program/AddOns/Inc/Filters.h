/**
 * @file Filters.h
 * @brief This file contains filters for the signals in the loop.
 *
 * This file provides filter configuration parameters and function
 * prototypes for signal processing within the control loop.
 *
 *
 * @author  Adam Katowski
 * @date    2026-01-17
 */


#ifndef INC_FILTERS_H_
#define INC_FILTERS_H_

#define AVG_FILT_SQ 5


/**
 * @brief Moving average filter.
 *
 * Computes the average value of the last N samples, where N is defined
 * by AVG_FILT_SQ.
 *
 * @param input Current input sample
 * @return Filtered output value
 */
float AvgFilter(float input);

#endif /* INC_FILTERS_H_ */
