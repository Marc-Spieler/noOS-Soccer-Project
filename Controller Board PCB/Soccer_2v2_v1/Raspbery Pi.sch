EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:My
LIBS:Soccer_2v2_v1-cache
EELAYER 25 0
EELAYER END
$Descr User 5236 4000
encoding utf-8
Sheet 19 21
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L GND #PWR0184
U 1 1 59D47450
P 3450 1450
F 0 "#PWR0184" H 3450 1200 50  0001 C CNN
F 1 "GND" H 3450 1300 50  0000 C CNN
F 2 "" H 3450 1450 50  0001 C CNN
F 3 "" H 3450 1450 50  0001 C CNN
	1    3450 1450
	1    0    0    -1  
$EndComp
Text HLabel 2400 1450 0    60   Input ~ 0
SPI0_MOSI
Text HLabel 2400 1550 0    60   Input ~ 0
SPI0_MISO
Text HLabel 2400 1650 0    60   Input ~ 0
SPI0_CLK
Text HLabel 2900 1650 2    60   Input ~ 0
SPI0_SS0
Text HLabel 2900 1550 2    60   Input ~ 0
GPIO25
Text HLabel 2400 1250 0    60   Input ~ 0
GPIO27
Text HLabel 2900 1350 2    60   Input ~ 0
GPIO23
Text HLabel 2400 1350 0    60   Input ~ 0
GPIO22
$Comp
L +5VD #PWR0185
U 1 1 5A0F1305
P 3450 1250
F 0 "#PWR0185" H 3450 1100 50  0001 C CNN
F 1 "+5VD" H 3450 1390 50  0000 C CNN
F 2 "" H 3450 1250 50  0001 C CNN
F 3 "" H 3450 1250 50  0001 C CNN
	1    3450 1250
	1    0    0    -1  
$EndComp
Text HLabel 2400 1150 0    60   Input ~ 0
GPIO17
Text HLabel 2900 1150 2    60   Input ~ 0
GPIO18
Wire Wire Line
	3450 1250 2900 1250
Wire Wire Line
	3450 1450 2900 1450
$Comp
L Conn_02x06_Odd_Even J16
U 1 1 5A460FB2
P 2600 1350
F 0 "J16" H 2650 1650 50  0000 C CNN
F 1 "Raspberry Pi" H 2650 950 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x06_Pitch2.54mm_SMD" H 2600 1350 50  0001 C CNN
F 3 "" H 2600 1350 50  0001 C CNN
	1    2600 1350
	1    0    0    -1  
$EndComp
$EndSCHEMATC
