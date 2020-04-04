/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.04.2020                                                  */
/************************************************************************/

#ifndef DATA_H
#define DATA_H

#include "asf.h"

typedef struct
{
    struct
    {
        struct 
        {
            Bool see;
            int8_t direction;
            uint8_t halfWidth;
        } ball;
        
        struct
        {
            Bool see;
            Bool haveClose;
            Bool haveFar;
            int8_t direction;
        } goal;
        
        uint8_t fps;
    } camera;
    
    float compass;
    
    struct
    {
        union
        {
            struct
            {
                uint16_t ls1   :1;
                uint16_t ls2   :1;
                uint16_t ls3   :1;
                uint16_t ls4   :1;
                uint16_t ls5   :1;
                uint16_t ls6   :1;
                uint16_t ls7   :1;
                uint16_t ls8   :1;
                uint16_t ls9   :1;
                uint16_t ls10  :1;
                uint16_t ls11  :1;
                uint16_t ls12  :1;
                uint16_t rsvd        :4;
            } single;
            uint16_t all;
        };
        
        struct
        {
            Bool see;
            int16_t esc;
            //int8_t diff;//uint
        };
    } line;
} sharedData_t;

extern sharedData_t data;

#endif
