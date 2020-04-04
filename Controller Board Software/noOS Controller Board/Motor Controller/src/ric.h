/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 30.03.2020                                                  */
/************************************************************************/

#ifndef RIC_H
#define RIC_H

#include "asf.h"

typedef struct
{
    struct
    {
        uint8_t heartbeat	 :1;
        uint8_t sleep_mode   :1;
        uint8_t rsvd		 :6;
    } ibit;
    
    uint8_t line_cal_value;
    uint8_t rsvd_1;
    uint32_t rsvd_2;
} motor2Sensor_t;

typedef struct
{
    struct
    {
        uint8_t heartbeat	 :1;
        uint8_t rsvd		 :7;
    } ibit;
    
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
            uint32_t see         :1;
            uint32_t esc         :9;
            uint32_t diff        :8;
            uint32_t rsvd        :14;
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
} sensor2Motor_t;

extern motor2Sensor_t m2s;
extern sensor2Motor_t s2m;

typedef struct
{
    uint32_t rsvd;
} motor2Raspberrypi_t;

typedef struct
{
    struct
    {
        uint16_t dir         :7;
        uint16_t see         :1;
        uint16_t diff        :5;
        uint16_t rsvd        :3;
    } goal;
    
    struct
    {
        uint16_t dir         :9;
        uint16_t see         :1;
        uint16_t have        :1;
        uint16_t have_2      :1;
        uint16_t rsvd        :4;
    } ball;
} raspberrypi2Motor_t;

extern motor2Raspberrypi_t m2r;
extern raspberrypi2Motor_t r2m;

void ricInit(void);
void ricMaintenance(void);
void prepareData2Send_Pi(void);

#endif
