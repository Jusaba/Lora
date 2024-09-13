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
    void TextoEnviadoaLora ( String cTexto );
    void MensajeConectadoaServerpic (void);
    void MensajeTxtRecibidodeLora ( String cTexto );
    void BorraPantallaTx (void);
    void BorraPantallaRx (void);
    void WritePantalla (void);
    void LimpiaPantalla (void);
    void VisualizaPantalla (void);
    void MensajeDispositivo (String cUtilizacion );
    void MensajeOn ();
    void MensajeOff ();
    void MensajeHora (int nSegundos, int nMinutos, int nHoras );


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
        
        LineasDisplay.Linea4 = "Lora->Serverpic";
        LineasDisplay.Linea5 = oMensajeLora.Remitente+"-:-"+oMensajeLora.Mensaje;
        LineasDisplay.Linea6 = "RSSI: " + (String) lora_rssi;
        WritePantalla();
 	}    

    void MensajeTxtEnviadoaLora ( Telegrama oMensajeLora )
    {
        LineasDisplay.Linea1 = "Serverpic->Lora";
        LineasDisplay.Linea2 = "Remitente: " + oMensajeLora.Remitente;
        LineasDisplay.Linea3 = "Mensaje: " + oMensajeLora.Mensaje;
        WritePantalla();
    }

    void TextoEnviadoaLora ( String cTexto )
    {
        LineasDisplay.Linea1 = "Dispositivo->LoraM";
        LineasDisplay.Linea2 = "Mensaje: " + cTexto;
        LineasDisplay.Linea3 = " ";
        WritePantalla();
    }
    void LimpiaPantalla (void)
    {
        display.clearDisplay();
    }
    void VisualizaPantalla (void)
    {
        display.display();
        display.setTextSize(1);
    }
    void MensajeDispositivo (String cUtilizacion )
    {
        display.setTextSize(2);
        LineasDisplay.Linea1 = cUtilizacion;
        display.setCursor(40, OLED_LINE1);     
        display.print(LineasDisplay.Linea1);
    }
    void MensajeOn ()
    {
        display.setTextSize(2); 
        LineasDisplay.Linea4 = "On";
        display.setCursor(45, OLED_LINE4);
        display.print(LineasDisplay.Linea4);
    }
    void MensajeOff ()
    {
        display.setTextSize(2); 
        LineasDisplay.Linea4 = "Off";
        display.setCursor(45, OLED_LINE4);
        display.print(LineasDisplay.Linea4);
    }

    void MensajeHora (int nSegundos, int nMinutos, int nHoras )
    {
        display.setTextSize(2);
        String cHoras = String(nHoras);
        String cMinutos = String(nMinutos);
        String cSegundos = String(nSegundos); 
        if (cHoras.length() == 1)
        {
            cHoras = "0"+cHoras;
        }
        if (cMinutos.length() == 1)
        {
            cMinutos = "0"+cMinutos;
        }
        if (cSegundos.length() == 1)
        {
            cSegundos = "0"+cSegundos;
        }

        LineasDisplay.Linea6 = cHoras+":"+cMinutos+":"+cSegundos;
        display.setCursor(15, OLED_LINE6);   
        display.print(LineasDisplay.Linea6);
    }

void MensajeOnTemporizado (int nSegundos, int nMinutos, int nHoras )
    {
        display.setTextSize(3); 
        LineasDisplay.Linea2 = "On";
        display.setCursor(40, OLED_LINE2);
        display.print(LineasDisplay.Linea2);
        display.setTextSize(2);
        String cHoras = String(nHoras);
        String cMinutos = String(nMinutos);
        String cSegundos = String(nSegundos); 
        if (cHoras.length() == 1)
        {
            cHoras = "0"+cHoras;
        }
        if (cMinutos.length() == 1)
        {
            cMinutos = "0"+cMinutos;
        }
        if (cSegundos.length() == 1)
        {
            cSegundos = "0"+cSegundos;
        }

        LineasDisplay.Linea5 = cHoras+":"+cMinutos+":"+cSegundos;
        display.setCursor(15, OLED_LINE5);   
        display.print(LineasDisplay.Linea5);
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