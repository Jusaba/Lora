/**
******************************************************
* @file LoraServerPic.h
* @brief Libreria para integrar los módulos Lora en Serverpic
* @author Julian Salas Bartolome
* @version 1.0
* @date 12/25/2022
*******************************************************/
#ifndef MENSAJES_H
    #define MENSAJES_H

    #include "LoraServerpic.h"

    void MensajeInicio (void)
    {
        display.clearDisplay();    
        display.setCursor(0, OLED_LINE1);
        display.print("Iniciando...");
        display.display();
    }

    void MensajeConectadoaServerpic ()
    {
        display.clearDisplay();    
        display.setCursor(0, OLED_LINE1);
        display.print("Conectado a ");
        display.setCursor(0, OLED_LINE3);
        display.print("Serverpic ");
        display.display();
    }
    
    void MensajeTxtRecibidodeLora ( String cTexto )
	{

		int lora_rssi = LoRa.packetRssi();
        display.clearDisplay();   
        /* Escreve RSSI de recepção e informação recebida */
        display.setCursor(0, OLED_LINE1);
        display.print("Soy lora2 ");
        display.setCursor(0, OLED_LINE2);
        display.print("RSSI: ");
        display.println(lora_rssi);
        display.setCursor(0, OLED_LINE3);
        display.print("Informacion: ");
        display.setCursor(0, OLED_LINE4);
        display.println(cTexto);
        display.display();     
	}    

#endif    