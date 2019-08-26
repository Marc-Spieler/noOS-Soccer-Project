/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 17.08.2019                                                  */
/************************************************************************/

#ifndef SENSOR_H
#define SENSOR_H

#include "asf.h"

typedef struct
{
    struct
    {
        int8_t dir;
        Bool see;
        uint8_t diff;
    } goal;
    
    struct
    {
        int8_t dir;
        Bool see;
        Bool have;
        Bool have_2;
    } ball;
    
    uint8_t camera_fps;

    struct
    {
        union
        {
            struct
            {
                uint16_t segment_1   :1;
                uint16_t segment_2   :1;
                uint16_t segment_3   :1;
                uint16_t segment_4   :1;
                uint16_t segment_5   :1;
                uint16_t segment_6   :1;
                uint16_t segment_7   :1;
                uint16_t segment_8   :1;
                uint16_t segment_9   :1;
                uint16_t segment_10  :1;
                uint16_t segment_11  :1;
                uint16_t segment_12  :1;
                uint16_t rsvd        :4;
            } single;
            uint16_t all;
        };
        
        struct
        {
            Bool see;
            int16_t esc;
            int8_t diff;//uint
        };
    } line;

    struct
    {
        struct
        {
            Bool arrived;
            Bool correction_dir;
        } one;
        
        struct
        {
            Bool arrived;
            Bool correction_dir;
        } two;
    } distance;

    struct
    {
        uint16_t voltage     :8;
        uint16_t percentage  :8;
    } battery;
    
    struct  
    {
        struct
        {
            float left;
            float right;
            float rear;
        } speed;
        
        struct
        {
            float left;
            float right;
            float rear;
        } output;
    } motor;
    
    float compass;
    Bool rpi_inactive;
} sensors_t;

extern sensors_t s;

void process_new_sensor_values(void);

#endif
