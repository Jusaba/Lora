/**
******************************************************
* @file IO.h
* @brief Documentacion de Modulo LoraTest wifi
* @author Julian Salas Bartolome
* @version 1.0
* @date 12/25/2022
*
*******************************************************/

#ifndef IO_H
	#define IO_H

    //Definicion de pines para la placa 
	#ifdef LoRa_32_V2
        int PinReset = 0;		
        // Definiciones para radio LORA 
 
	#endif	
   	#ifdef TUF2000M
        int RX_PIN = 17; // connect to converter's RX wire
        int TX_PIN = 23; // connect to converter's TX wire
    #endif

#endif