/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 07.07.2019                                                  */
/************************************************************************/

#ifndef BT_H
#define BT_H

#include "asf.h"

void bt_init(void);
void bt_write(uint8_t *pbuf, uint8_t len);
uint8_t bt_read(uint8_t *pbuf, uint8_t maxcount);

#endif
