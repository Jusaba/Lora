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
    #include "Servidor.h"
    

	typedef struct {
		String Linea1;			
		String Linea2;		
		String Linea3;	
		String Linea4;			
		String Linea5;		
		String Linea6;				
	}Pantalla;

    
    Pantalla LineasDisplay;

    void MensajeInicio (void);
    void MensajeTxtEnviadoaLora ( Telegrama oMensajeLora );
    void MensajeConectadoaServerpic (void);
    void MensajeTxtRecibidodeLora ( String cTexto );
    void BorraPantallaTx (void);
    void BorraPantallaRx (void);
    void WritePantalla (void);

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
    
    void MensajeTxtRecibidodeLora (Telegrama oMensajeLora )
	{

		int lora_rssi = LoRa.packetRssi();
        
        String cDestinatario = String (oMensajeLora.Mensaje).substring(0,  String(oMensajeLora.Mensaje).indexOf("-:-") );
        LineasDisplay.Linea4 =  cDestinatario+"->LoraM";
        LineasDisplay.Linea5 = oMensajeLora.Remitente+"-:-"+oMensajeLora.Mensaje;
        LineasDisplay.Linea6 = "RSSI: " + (String) lora_rssi;
        WritePantalla();
 	}    

    void MensajeTxtEnviadoaLora ( Telegrama oMensajeLora )
    {
        String cDestinatario =  String (oMensajeLora.Mensaje).substring(0,  String(oMensajeLora.Mensaje).indexOf("-:-") );
        String cMensaje = String(oMensajeLora.Mensaje).substring(  3 + String(oMensajeLora.Mensaje).indexOf("-:-"),  String(oMensajeLora.Mensaje).length() ); 
        LineasDisplay.Linea1 = "LoraM->"+cDestinatario;
        LineasDisplay.Linea2 = "Remitente: " + oMensajeLora.Remitente;
        LineasDisplay.Linea3 = "Mensaje: " + cMensaje;
        WritePantalla();
    }


    void WritePantalla (void)
    {
        display.clearDisplay();
        display.setCursor(0, OLED_LINE1);
        display.print(LineasDisplay.Linea1);
        display.setCursor(0, OLED_LINE2);
        display.print(LineasDisplay.Linea2);
        display.setCursor(0, OLED_LINE3);
        display.print(LineasDisplay.Linea3);
        display.setCursor(0, OLED_LINE4);
        display.print(LineasDisplay.Linea4);
        display.setCursor(0, OLED_LINE5);
        display.print(LineasDisplay.Linea5);
        display.setCursor(0, OLED_LINE6);
        display.print(LineasDisplay.Linea6);
        display.display();     
    }
#endif    