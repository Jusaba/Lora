/**
******************************************************
* @file LoraServerPic.h
* @brief Libreria para integrar los módulos Lora en Serverpic
* @author Julian Salas Bartolome
* @version 1.0
* @date 12/25/2022
*******************************************************/
/*****************************************************
 * NOTAS IMPORTANTES
 * - Se ha tenido que declarar DIO como pin 26 en las definiciones para Radio Lora
 *   y al iniciar la radio se ha de sustituir LORA_DEFAULT_DIO0_PIN (GPIO2 2) por DIO0 (GPIO26)
*/
#ifndef LORASERVERPIC_H
    #define LORASERVERPIC_H

        #define ESP32

        #include <Wire.h>

        //Pantalla
        #include <Adafruit_GFX.h>
        #include <Adafruit_SSD1306.h>
        // Lora
        #include <LoRa.h>
        #include <SPI.h>

		//#include <GLobal.h>
        #include <Servidor.h>

		//String
		#include <stdint.h>
		#include <iostream>
		#include <string>		
        
		Telegrama oLoraMensaje;			 	//Telegrama correspondiente a la informacion recibida por radio
		Telegrama oLoraMensajeRepeticion;
		boolean lErrorRxLora = 0;			//Flag para indicar que se ha recibido información erronea
   
		#define DebugLora

        //------------------------------------
	    //Declaracion de variables Particulares
	    //------------------------------------
		//------------------------------
		// Definiciones de radio Lora
		//------------------------------
		//-----------------------------
		// Definiciones para radio LORA
		//----------------------------- 
		#define HIGH_GAIN_LORA     20  /* dBm */
		#define BAND               866E6  /* 866MHz de frequencia */

        #define SCK_LORA           5
        #define MISO_LORA          19
        #define MOSI_LORA          27
        #define RESET_PIN_LORA     14
        #define SS_PIN_LORA        18
		#define DIO0	   		   26

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



	   
        //--------------------------------------------
	    //Declaracion de funciones Particulares
	    //----------------------------------------------
        Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

	    //----------------------------------------------
    	//Declaracion de funciones Particulares
	    //----------------------------------------------

        void DisplayInit(void); 
       
	   	bool LoraInit(void); 
		void StringToLora ( String cMensaje );
		//void StringToLora ( std::string cMensaje );
		void TelegramaToLora (Telegrama oTelegrama);
		void RepiteTelegramaToLora (void);
		String LoraToString ( int nBytes );
		Telegrama StringToTelegrama (String cTexto);
		void onReceive(int packetSize);
		void onTxDone(void);

		uint16_t calcularCRC(const char *cadena);
		String  uint16ToAsciiHex(uint16_t nNumero);
		uint16_t AsciiHexTouint16 (String cAsciiHex );
	
	/* FUncion que inicializa el display OLED 
	*/ 
	void DisplayInit(void) 
	{ 
	   if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) 
	   { 
			#ifdef DebugLora
		   		Serial.println("Fallo inicializando comunicacion con  OLED");  
			#endif	
	   } else {
			#ifdef DebugLora
		   		Serial.println("Iniciada la comunicacion con OLED"); 
		    #endif
		   /* Limpia el display y configura la fuente */ 
		   display.clearDisplay(); 
		   display.setTextSize(1); 
		   display.setTextColor(WHITE); 
	   } 
	}  

	/**
	* @brief Funcion que inicia la comunicacion con la radio LORA
 	* @return: devuelve el estado de la conexion, 'true' conectado con exito, en caso contrario 'false'
	*/
	bool LoraInit (void)
	{
		bool Salida = false;
		#ifdef DebugLora
			Serial.println("Inicializando Lora...");
		#endif
		SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, SS_PIN_LORA);
		LoRa.setPins(SS_PIN_LORA, RESET_PIN_LORA, DIO0);
			
		if (!LoRa.begin(BAND)) 
		{
			#ifdef DebugLora
		    	Serial.println("Fallo en inicio de Lora. reintento en 1 segundo...");        
			#endif
		    delay(1000);
		    Salida = false;
		}else{
		    /* Configura LoRa para 20dBm, maxima potencia/sensibilidad posible */ 
		    LoRa.setTxPower(HIGH_GAIN_LORA); 
			#ifdef DebugLora
			    Serial.println("Lora iniciado correctamente");
			#endif
		    Salida = true;
		}
 		LoRa.onReceive(onReceive);				//Rutina de atencion de recepciobn
		LoRa.onTxDone(onTxDone);				//Rutina de atencion de fin transmision
		return Salida;
	}	


	/**
	* @brief Funcion que  manda un texto por Lora, le añade el CRC separado por una coma
 	* @param cMensaje String a mandar por radio
 	*          
	*/
	void StringToLora ( String cMensaje )
	{
    	int nLongitud = cMensaje.length();							//Longitud del mensaje
		const char *p_Mensaje = cMensaje.c_str();
		uint16_t crc = calcularCRC(p_Mensaje);


		LoRa.idle();												//Pone la radio en espera
		LoRa.beginPacket();											//Iniciamos la tranmisión
		LoRa.print(cMensaje);
		LoRa.print(",");											//Separador de CRC
		LoRa.print(uint16ToAsciiHex(crc));							//Inclusión de CRC
    	LoRa.endPacket(true);										//Cerramos la transmision
		#ifdef DebugLora
			Serial.println ("StringToLora-----------------");
			Serial.print ("Enviado mensaje de ");
			Serial.print (nLongitud);
			Serial.println (" bytes ");
			//Serial.println ((String) aTexto);
			Serial.println (cMensaje);
			Serial.println ("-----------------------------");
		#endif
	}
	/**
	******************************************************
	* @brief Manda un Telegrama de Serverpic por Lora como un String
	*
	* @param oTelegrama.- Telegrama de Serverpic a enviar por Lora
	*
	*/
	void TelegramaToLora (Telegrama oTelegrama)
	{
		String cTexto = oTelegrama.Remitente+"-:-"+oTelegrama.Mensaje;			//Pasamos la estructura mensaje a un string formato 'remitente-:-orden'
		oLoraMensajeRepeticion = oTelegrama;
		StringToLora(cTexto);	
	}

	/**
	******************************************************
	* @brief Repite el ultimo telegrama enviado por Lora
	*
	* @param oTelegrama.- Telegrama de Serverpic a enviar por Lora
	*
	*/
	void RepiteTelegramaToLora (void)
	{
		String cTexto = oLoraMensajeRepeticion.Remitente+"-:-"+oLoraMensajeRepeticion.Mensaje;			//Pasamos la estructura mensaje a un string formato 'remitente-:-orden'
		StringToLora(cTexto);	
	}

	/**
	* @brief Funcion que  se ejecuta cuando se recibe un paquete
 	* @param packetSize numero de bytes recibidos. Este parametro lo pasa la libreria
 	* 
	*/
	void onReceive(int packetSize) {
  		String cTexto = "";																							//Definimos cTexto
		lErrorRxLora = 0;
 		while (LoRa.available()) {																					//Leemos caracter a caracter el buffer de recepcion
    		cTexto += (char)LoRa.read();																			//Le vamos añadiendo los caracteres a cTexto
  		}
		if ( cTexto.indexOf("-:-") > -1)																			//Si el mensaje contiene la cadena '-:-' es que es un mensaje del sistema
		{
			#ifdef DebugLora
				Serial.println ("onReceive-----------------");
				Serial.println ("Recibido mensaje por radio ");
				Serial.println (cTexto);
				Serial.println ("-----------------------------");			
			#endif
			String cMensaje = String(cTexto).substring(0, String(cTexto).indexOf(","));								//Separamos mensaje y CRC para comprobar si el mensaje es correcto
			String cCRC = String(cTexto).substring ( 1 + String(cTexto).indexOf(","), String(cTexto).length() );	
			const char *p_Mensaje = cMensaje.c_str();
			if ( calcularCRC(p_Mensaje) ==  AsciiHexTouint16 (cCRC ))												//Si el mensaje es correcto lo trasnformamos en estructura Telegrama
			{
				oLoraMensaje=StringToTelegrama(cMensaje);
			}else{																									//Si no es correcto	
				lErrorRxLora = 1;																					//Ponemos el flag de error en mensaje a 1
			}
		}else{																										//Si no es mensaje de Sistema....
			if ( cTexto.indexOf("Ko") > -1)																			
			{

			}else{
				if ( cTexto.indexOf("Ok") > -1)
				{
				
				}else{
					#ifdef DebugLora
						Serial.println ("onReceive-----------------");
						Serial.println ("Recibido ruido ");
						Serial.println ("-----------------------------");
					#endif
				}
			}	
		}
 	}

	/**
	* @brief Funcion convierte un String formato serverpic 'Remitente-:-Mensaje' a Telegrama
 	* @param cTexto String a convertir
 	* 
	* @return Devuelve el telegrama resultante         
	*/
	Telegrama StringToTelegrama (String cTexto)
	{

		Telegrama oTelegrama;
		oTelegrama.Remitente = String(cTexto).substring( 0, String(cTexto).indexOf("-:-") );
	    if ( (oTelegrama.Remitente).indexOf("OK") == 0 )                                      //Cuando se recibe OK desde el servidor por una accion bien realizada, solo se recibe OK, no se recibe remitente
      	{                                                                                     //El OK lo envia solo y por tanto es el campo 0 del mensaje recibido y se coloca en remitente
          oTelegrama.Remitente = "Servidor";
          oTelegrama.Mensaje = "OK";   
          oTelegrama.lRxMensaje = 0;
      }else{
        oTelegrama.Mensaje = String(cTexto).substring(  3 + String(cTexto).indexOf("-:-"),  String(cTexto).length() );
      	oTelegrama.lRxMensaje = 1;
	  }	
	  #ifdef DebugLora
	  
			Serial.println ("StringToTelegrama-----------------");
			Serial.println ("Creado telegrama de ");
			Serial.println ( cTexto );
			Serial.print ("Remitente: ");
			Serial.println(oTelegrama.Remitente);
			Serial.print ("Mensaje: ");
			Serial.println(oTelegrama.Mensaje);
			Serial.print ( "lRxMensaje: ");
			Serial.println ( (oTelegrama.lRxMensaje) ? "1": "0");
			Serial.println ("-----------------------------");
	  #endif	
	  return oTelegrama;
	}
	/**
	* @brief Funcion que  se ejecuta cuando se termina la transmisión de un paquete
 	* 
	*/
	void onTxDone() {
  		LoRa.receive(); 		//Pone la radio en recepcion
	}
	/**
	* @brief Funcion que  calcula CRC de una cadena
 	* 
	*/
	uint16_t calcularCRC(const char *cadena) {

	    uint16_t crc = 0xFFFF; // Valor inicial
	    while (*cadena) {
	        crc ^= *cadena++;
	        for (int i = 0; i < 8; i++) {
	            if (crc & 0x0001)
	                crc = (crc >> 1) ^ 0xA001;
	            else
	                crc >>= 1;
	        }
	    }
	    return crc;
	}
	
	/**
	* @brief Funcion que  convierte un uint16_t a String HEX
 	* 
	*/
	String uint16ToAsciiHex ( uint16_t nNumero ) 
	{
	    const char hexDigits[] = "0123456789ABCDEF";
		String cTxtHex = String(' ');
	
	    for (int i = 0; i < 4; ++i) {
	        cTxtHex =  hexDigits[nNumero & 0x0F] + cTxtHex;
	        nNumero >>= 4;
	    }
	
	    cTxtHex = cTxtHex + '\0'; // Agregar el carácter nulo al final

		return (cTxtHex);
	}

	/**
	* @brief Funcion que  convierte un  String HEX a uint16_t
 	* 
	*/
	uint16_t AsciiHexTouint16 (String cAsciiHex )
	{
	 uint16_t result = (uint16_t) strtol(cAsciiHex.c_str(), NULL, 16);
    	return result;
	}



#endif

	/**
	* 
 	* Esta funcion es alternativa a la utilizada, en lugar de usar LoRa.print
 	* utiliza LoRa.write. Es necesario conversion de std::string a array de char   
	* Para llamar a la función se debe hacer de la siguiente forma
	*      		//StringToLora(std::string(cTexto.c_str()));								//Mandamos el String por Lora
	* Siendo cTexto variable String
	*/
	/*
	void StringToLora ( std::string cMensaje )
	{
    	int nLongitud = cMensaje.length();							//Longitud del mensaje
    	std::string cTexto = cMensaje;								//Pasamos el mensaje a la variable cTexto
    	char aTexto[cTexto.length() + 1];							//Creamos array con capacidad para almacenar el mensaje
    	strcpy(aTexto, cTexto.c_str());								//Copiamos el mensaje enb el array
		LoRa.idle();
		LoRa.beginPacket();											//Iniciamos la tranmisión
    	LoRa.write((unsigned char *)&aTexto, sizeof(aTexto));		//Transmitimos el mensaje	
		LoRa.endPacket(true);										//Cerramos la transmision
	}
	*/