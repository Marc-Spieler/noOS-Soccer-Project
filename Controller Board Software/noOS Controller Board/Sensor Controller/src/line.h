/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 07.03.2019                                                  */
/************************************************************************/

#ifndef LINE_H
#define LINE_H

#include "asf.h"

void dacc_init(void);
void update_line_calibration_value(uint16_t calibration_value);
void update_line_values(void);
void calculate_line_esc_direction(void);

#endif
