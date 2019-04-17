/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 30.03.2019                                                  */
/************************************************************************/

#ifndef SD_H
#define SD_H

#include "asf.h"

void sd_init(void);
void open_file_match(void);
void close_file_match(void);
void write_data(void);
void write_time_test(void);

#endif
