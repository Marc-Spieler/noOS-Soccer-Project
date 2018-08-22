/************************************************************************/
/* Author: Tobias Bungard                                               */
/* Team: noOS                                                           */
/* Created: 01.01.18                                                    */
/************************************************************************/


#ifndef PACKETS_H
#define PACKETS_H

struct Info
{
	struct
    {
		uint8_t x                   :4;
		uint8_t y                   :4;
	} roboPos1;
	
	struct
    {
		uint8_t x                   :4;
		uint8_t y                   :4;
	} roboPos2;
	
	struct
    {
		uint8_t x                   :4;
		uint8_t y                   :4;
	} ballPos;
	
	struct
    {
		uint32_t horizontal         :12;
		uint32_t vertical           :9;
	} ball1;
	
	struct
    {
		uint32_t horizontal         :12;
		uint32_t vertical           :9;
	} ball2;
	
	struct
    {
		struct
        {
			uint32_t horizontal     :12;
			uint32_t vertical       :9;
		} ball0;
		
		struct
        {
			uint32_t horizontal     :12;
			uint32_t vertical       :9;
		} ball1;
		
		struct
        {
			uint32_t horizontal     :12;
			uint32_t vertical       :9;
		} ball2;
		
		struct
        {
			uint32_t horizontal     :12;
			uint32_t vertical       :9;
		} ball3;
		
		struct
        {
			uint32_t horizontal     :12;
			uint32_t vertical       :9;
		} ball4;
		
	} history;
	
	
	struct
    {
		uint8_t s1                  :1;
		uint8_t s2                  :1;
		uint8_t valid1              :1;
		uint8_t valid2              :1;
		uint8_t have1               :1;
		uint8_t have2               :1;
	} status;
	
	bool wlanOld = true;
	bool wlanNew = true;
	bool go = false;
};

struct PacketBluetooth
{
	uint16_t id;
	
	struct
    {
		uint8_t x                   :4;
		uint8_t y                   :4;
	} roboPos;
	
	struct
    {
		uint32_t horizontal         :12;
		uint32_t vertical           :9;
		uint32_t onField            :1;
		uint32_t valid              :1;
		uint32_t have               :1;
		uint32_t rsv                :8;
	} bits;
};
 
struct PacketSPI_OUT
{
	struct
    {
		uint8_t x                   :4;
		uint8_t y                   :4;
	} roboPos;
	
	struct
    {
		uint8_t x                   :4;
		uint8_t y                   :4;
	} ballPos;
	
	struct
    {
		uint16_t ball               :12;
		uint16_t have               :1;
		uint16_t rsvd               :3;
	} bits;
	
};

struct PacketSPI_IN
{
	struct
    {
		uint8_t x                   :4;
		uint8_t y                   :4;
	} roboPos;
	
	struct
    {
		uint8_t WLAN                :1;
		uint8_t onField             :1;
		uint8_t have                :1;
		uint8_t validX              :1;
		uint8_t validY              :1;
		uint8_t rsvd                :3;
	} bits;
	
	struct
    {
		uint16_t compass            :12;
		uint16_t rsvd               : 4;
	} info;
};

#endif
