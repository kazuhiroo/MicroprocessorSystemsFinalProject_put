/*
  * @file    : Filters.h
  * @author  : a.katowski, https://github.com/kazuhiroo
  * @date    : Jan 17, 2026
  * @brief   : Implemented filters for the signals in the loop
  *
*/

#ifndef INC_FILTERS_H_
#define INC_FILTERS_H_

#define AVG_FILT_SQ 10


/*
 * @brief   : moving average filter defined by the samples quantity
 *
 * */
float AvgFilter(float input);

#endif /* INC_FILTERS_H_ */
