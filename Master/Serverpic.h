/**
******************************************************
* @file ServerPic.h
* @brief Documentacion de Modulo LoraTest wifi
* @author Julian Salas Bartolome
* @version 1.0
* @date 12/25/2022
*******************************************************/

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
		#define Ino "LoraTest"					
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
	//DISPLAY
	//----------------------------------------------
	#define Display
	//----------------------------------------------
	//DEBUG
	//----------------------------------------------
	#define Debug
	
    //----------------------------------------------
	//HOME KIT
	//----------------------------------------------
	//#define HomeKit
	
    //----------------------------------------------
	//Web Socket
	//----------------------------------------------
	#define WebSocket	
	
    //----------------------------------------------
	//Teimpo de Test de conexion
	//----------------------------------------------
	#define TiempoTest	15000												//Tiempo en milisegundos para Test de conexion a servidor


	//----------------------------------------------
	//Declaracion de funciones Universales
	//----------------------------------------------
	
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
	
	 //Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
	
	//----------------------------
	//Funciones Particulares
	//----------------------------	



	

	



	
#endif
