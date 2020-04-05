/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.04.2020                                                  */
/************************************************************************/

#ifndef COMPASS_H
#define COMPASS_H

#include "asf.h"

void compassInit(void);
void compassMaintenance(void);
uint8_t get_compass_cal_status(uint8_t *cal_status);
void setGoalReference(Bool inverted);
void compassCalibrationStep(void);
void setCompassIsBusy(void);
uint8_t compass_is_busy(void);

#endif
