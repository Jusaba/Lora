/**
******************************************************
* @file ServerPic.h
* @brief Documentacion de Modulo LoraTest wifi
* @author Julian Salas Bartolome
* @version 1.0
* @date 12/25/2022
*******************************************************/
//https://vasanza.blogspot.com/2021/08/esp32-real-time-clock-rtc-interno.html

#ifndef SERVERPIC_H
	#define SERVERPIC_H

	#define ESP32
	//----------------------------------------------
	//Includes Universales
	//----------------------------------------------
	    //Librerias LoRa 32
    //Aruino & ESP32
    #include <WebServer.h>
    #include "esp_camera.h"
    #include <WiFi.h>
    #include "img_converters.h"
    #include "Arduino.h"
    #include "soc/soc.h"           
    #include "soc/rtc_cntl_reg.h"  
    #include "driver/rtc_io.h"
    #include <WebServer.h>
    
    //#include <StringArray.h>
    #include <SPIFFS.h>
    #include <FS.h>
    #include <WiFiUdp.h>
    #include <ArduinoOTA.h>
    #include <Wire.h>
    #include <base64.h>
    #include <libb64/cencode.h>
    
    //BLE
    #include <BLEDevice.h>
    #include <BLEUtils.h>
    #include <BLEScan.h>
    #include <BLEAdvertisedDevice.h>
    #include <BLEEddystoneURL.h>
    #include <BLEEddystoneTLM.h>
    #include <BLEBeacon.h>
    
	#include "LoraServerpic.h"

 	// Lora
    #include <LoRa.h>
    #include <SPI.h>
    #include <Wire.h>

    //Pantalla
    #include <Adafruit_GFX.h>
    #include <Adafruit_SSD1306.h>
    //Fin librerias LoRa 32
    
	#include <ESP32Time.h>

	#include "Mensajes.h"
	

	#include "Global.h"
	#include "Configuracion.h"
	#include "Eprom.h"
	#include "ModoAP.h"
	#include "ModoSTA.h"
	#include "Servidor.h"

	//----------------------------------------------
	//Includes Particulares
	//----------------------------------------------

	//------------------
	//Hardware Utilizado 
	//------------------	

	
    #define LoRa_32_V2

	#ifdef LoRa_32_V2
		#define Placa "LoRa 32 V2"
		#define Modelo ","
		#define Ino "Slave_Rele"					
		//------------
		//Pulsador-LED 
		//------------	
		//#define PulsadorLed
		//------------
		//Logica
		//------------	
		#define Logica	1
		//-----------------
		//TiEmpo de rebotes
		//-----------------
 		//#define TempoRebotes 150
			

	#endif

	
	//----------------------------
	//CARACTERISTICAS DISPOSITIVO
	//----------------------------
	#define VIno "1.0"						//Version del programa principal
	#define VBuild  "1"                     //Numero de compilacion

	//---------------------------------
	//CARACTERISTICAS DE LA COMPILACION
	//---------------------------------
	#define Compiler "VSCode Arduino-Cli";	//Compilador
	#define VCore "0.0.6";					//Versión del Core Arduino

	#include "IO.h";


	//----------------------------------------------
	//DEBUG
	//----------------------------------------------
	#define Debug
	
	//----------------------------------------------
	//Display
	//----------------------------------------------
	#define Display


    //----------------------------------------------
	//HOME KIT
	//----------------------------------------------
	//#define HomeKit
	
    //----------------------------------------------
	//Web Socket
	//----------------------------------------------
	#define WebSocket	
	
    //----------------------------------------------
	//Tiempo de Test de conexion
	//----------------------------------------------
	#define TiempoTest	120000												//Tiempo en milisegundos para Test de conexion a servidor

    //----------------------------------------------
	//Declaracion reloj tiempo real
	//----------------------------------------------
	ESP32Time rtc;
	


	//----------------------------------------------
	//Declaracion de funciones Universales
	//----------------------------------------------
	boolean GetDispositivo (void);																//Devuelve el estado del sipositivo
	void DispositivoOff (void);																	//Pone el dispositivo en Off
	void DispositivoOn (void);																	//Pone el dispositivo en On

	//----------------------------------------------
	//Declaracion de funciones RTC
	//----------------------------------------------
	void SetHora ( int nSg, int nMinutos, int nHora );											//Pone la hora, minutos y segundos en RTC
	void SetFecha ( int nDia, int nMes, int nAno );												//Pone Dia, Mes y año en RTC
	void SetHoraFecha (  int nSg, int nMinutos, int nHora, int nDia, int nMes, int nAno );		//Pone hora, minutos, segundos, dia, mes y año en RTC

	//----------------------------------------------
	//Declaracion de funciones Display
	//----------------------------------------------	
	void SegundosToHHMMSS ( int nSegundosTot );													//Visualiza en pantalla una cantidad de segundos en HH:MM:SS					


	//----------------------------------------------
	//Declaracion de funciones Particulares
	//----------------------------------------------

	//------------------------------------
	//Declaracion de variables Universales
	//------------------------------------
	WebServer server(80);       											//Definimos el objeto Servidor para utilizarlo en Modo AP
	WiFiClient Cliente;														//Definimos el objeto Cliente para utilizarlo en Servidor
	Telegrama oMensaje;									 					//Estructura mensaje donde se almacenaran los mensajes recibidos del servidor

	String cSalida;															//Variable donde se deja el estado ( String ) para mandar en mensaje a ServerPic
	boolean lEstado = 0;													//Variable donde se deja el estado del dispositivo para reponer el estado en caso de reconexion
	boolean lConexionPerdida = 0;											//Flag de conexion perdida, se pone a 1 cuando se pierde la conexion. Se utiliza para saber si se ha perdido la conexion para restablecer estado anterior de la perdida


	boolean lHomeKit;													    //Flag para habililtar HomeKit ( 1 Habilitado, 0 deshabilitado )
	boolean lTelegram;														//Flag para habililtar Telegram ( 1 Habilitado, 0 deshabilitado )
	boolean lPush ;															//Flag para habililtar lPush ( 1 Habilitado, 0 deshabilitado )
	boolean lWebSocket ;  													//Flag para habililtar WebSocket ( 1 Habilitado, 0 deshabilitado )
	boolean lEstadisticas;													//Flag para habilitar Estadisticas ( 1 Habilitado, 0 Deshabilitado )
 
	long nMiliSegundosTest = 0;												//Variable utilizada para temporizar el Test de conexion del modulo a ServerPic
	long nMilisegundosRebotes = 0;                							//Variable utilizada para temporizar el tiempo de absorcion de rebotes
	String cDispositivo = String(' ');										//Variable donde se deja el nombre del dsipositivo. Se utiliza para comunicar con HomeKit
	String cPush = String(' ');												//Cliente de push y Telegram
	
	boolean lFlagInterrupcion = 0;                							//Flag para indicar a loop() que ha habido pulsacion

	//------------------------------------
	//Declaracion de variables Particulares
	//------------------------------------

	int nSegundosTime = 0;													//Variable donde se almacenan los segundos reales de RTC en la comprobacion anterior 
	int nSegundosOn = 0;													//Variable con los segunods de temporizacion
	int nSegundosCiclo = 0;													//Variable donde se almacena los segundos actuales del RTC
	int nSegundosCicloDif = 0;												//Diferencia de segundos entre el momento actual y la comprobacion anterior
	boolean lTemporizado = 0;												//Flag para indicar si hay un proceso temporizado ( On por ejemlo )

	boolean lInicio = 0;													//Flag que indica al loop que está en su primer ciclo de loop, estará a 1 cuando se inicia el cliente
	boolean lBroadcast = 0;													//Flag que se pone a 1 cuando se recibe mensaje de Broadcast				

	String cDispositivoMaster = String(' ');								//Variable donde se deja el nombre del dsipositivo maestro

		
	//------------------------------
	// Definiciones de pantalla OLED 
	//------------------------------
    #define OLED_SDA_PIN    4
    #define OLED_SCL_PIN    15
    #define OLED_RESET      16
	#define SCREEN_WIDTH    128 
	#define SCREEN_HEIGHT   64  
	#define OLED_ADDR       0x3C 
    // Lineas del display
	#define OLED_LINE1     0
	#define OLED_LINE2     10
	#define OLED_LINE3     20
	#define OLED_LINE4     30
	#define OLED_LINE5     40
	#define OLED_LINE6     50


	
	
	//Variables donde se almacenan los datos definidos anteriormente para pasarlos a Serverpic.h
	//para mandar la información del Hardware y Software utilizados
	//En la libreria ServerPic.h estan definidos como datos externos y se utilizan en la funcion
	//Mensaje () para responder al comando generico #Info.
	//Con ese comando, el dispositivo mandara toda esta información al cliente que se lo pida
	// ESTOS DATOS NO SON PARA MODIFICAR POR USUARIO
	//----------------------------
	//Datos del programa principal
	//----------------------------
	String VMain = VIno;
	String Main = Ino; 
	String VModulo = VBuild;
	//----------------------------
	//Datos de compilación
	//----------------------------	
	String Compilador = Compiler;
	String VersionCore = VCore;

	//----------------------------
	//Datos de Hardware
	//----------------------------	
	String ModeloESP = Modelo;
	String Board = Placa;


	//----------------------------
	//Funciones Universales
	//----------------------------	
	/**
	******************************************************
	* @brief Devielve el estado del dispositivo
	*
	* @return devuelve <b>1</b> si el dispositivo esta conectado o <b>0<\b> en caso de que no este conectado
	*/
	boolean GetDispositivo (void)
	{
		return (lEstado);
    }   
	/**
	******************************************************
	* @brief Pone el dispositivo en On
	*
	*/
	void DispositivoOn (void)
	{
		lEstado = 1;
		MensajeOn();
	}
	/**
	******************************************************
	* @brief Pone el dispositivo en OPff
	*
	*/
	void DispositivoOff (void)
	{
		lEstado = 0;
		MensajeOff();
	}   
	//----------------------------
	//Funciones RTC
	//----------------------------	
	/**
	******************************************************
	* @brief Pone Hora, Minutos y Segundos en RTC
	*
	* @param nSg.- Segundos
	* @param nMinutos.- Minutos
	* @param nHora.- Horas
	*/
	void SetHora ( int nSg, int nMinutos, int nHora )
	{
		int nDia = rtc.getDay();									  	//Salvamos el dia registrado en RTC
		int nMes = rtc.getMonth();										//Salvamos el mes registrado en RTC
		if ( nMes == 0 )
		{
			nMes = 1;
		}
		int nAno = rtc.getYear();										//Salvamos el año registrado en RTC
		#ifdef Debug
			Serial.print ("Dia: ");
			Serial.println ( nDia );
			Serial.print ("Mes: ");
			Serial.println ( nMes );
			Serial.print ("Año: ");
			Serial.println ( nAno );
		#endif
		SetHoraFecha ( nSg, nMinutos, nHora, nDia, nMes, nAno ); 	    //Grabamos en RTC las horas, minutos y segundos pasados junto a los datos de fecha salvados
	} 
	/**
	******************************************************
	* @brief Pone Dia, Mes y Año en RTC
	*
	* @param nDia.- Dia
	* @param nMes.- Mes
	* @param nAno.- Año
	*/	
	void SetFecha ( int nDia, int nMes, int nAno )
	{
		int nSg = rtc.getSecond();										//Salvamos los segundos registrados en RTC
		int nMinutos = rtc.getMinute();									//Salvamos los minutos	registrados en RTC
		int nHora = rtc.getHour(true);									//Salvamos la hora registrada en RTC
		#ifdef Debug
			Serial.print ("Segundos: ");
			Serial.println ( nSg );
			Serial.print ("Minutos: ");
			Serial.println ( nMinutos );
			Serial.print ("Hora: ");
			Serial.println ( nHora );
		#endif	
		SetHoraFecha ( nSg, nMinutos, nHora, nDia, nMes, nAno ); 		//Grabamos en RTC el dia, mes y año pasados junto a los datos de hora salvados
	}
	/**
	******************************************************
	* @brief Pone Hora, Minutos, Segundos, dia, mes y año en RTC
	*
	* @param nSg.- Segundos
	* @param nMinutos.- Minutos
	* @param nHora.- Horas
	* @param nDia.- Dia
	* @param nMes.- Mes
	* @param nAno.- Año
	*/
	void SetHoraFecha (  int nSg, int nMinutos, int nHora, int nDia, int nMes, int nAno )
	{
		rtc.setTime ( nSg, nMinutos, nHora, nDia, nMes, nAno );
	}
	/**
	******************************************************
	* @brief Convierte un numero de segundos en formato HH:MM:SS y lo manda a una funcion para visualizarlo en pantalla
	*
	* @param nSegundosTot.- Segundos 
	*/
	void SegundosToHHMMSS ( int nSegundosTot )
	{
		int nSegundosTemp = nSegundosTot;
		int nHoras = nSegundosTemp / 3600;
		nSegundosTemp = nSegundosTemp - ( nHoras * 3600 );
		int nMinutos = nSegundosTemp / 60;
		int nSegundos = nSegundosTemp - ( nMinutos * 60 );
		#ifdef Display
			MensajeOnTemporizado ( nSegundos, nMinutos, nHoras );
		#endif	
	}
	//----------------------------
	//Funciones Particulares
	//----------------------------



	

	



	
#endif
