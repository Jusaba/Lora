/**
******************************************************
* @file LoraTest.ino
* @brief LoraTest
* @author Julian Salas Bartolome
* @version 1.0
* @date 12/25/2022
*
*
*******************************************************/

#include "ServerPic.h"
#include "IO.h"




void setup() {

	Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);												//Inicializamos 1Wire
    DisplayInit();																		//Inicializamos la pantalla


  	#ifdef Debug																		//Usamos el puereto serie solo para debugar	
		Serial.begin(9600);																//Si no debugamos quedan libres los pines Tx, Rx para set urilizados
		Serial.println("Iniciando........");
	#endif
	EEPROM.begin(2568);																	//Reservamos zona de EEPROM
	//BorraDatosEprom ( 0, 256 );														//Borramos 128 bytes empezando en la posicion 0		

  	//Declaracion de Entradas/Salidas
	//pinMode(PinReset, INPUT_PULLUP);           											//Configuramos el pin de reset como entrada 

	MensajeInicio();

	if ( LeeByteEprom ( FlagConfiguracion ) == 0 )										//Comprobamos si el Flag de configuracion esta a 0
	{																					// y si esta
		ModoAP();																		//Lo ponemos en modo AP
	}else{																				//Si no esta
		if ( ClienteSTA() )																//Lo poenmos en modo STA y nos conectamos a la SSID
		{																				//Si jha conseguido conectarse a ls SSID en modo STA
	        if ( ClienteServerPic () )													//Intentamos conectar a ServerPic
    		{
				CheckFirmware();    													//Comprobamos si el firmware esta actualizado a la ultima version
		    	#ifdef Debug
        			Serial.println(" ");
        			Serial.println("Conectado al servidor");
      			#endif 
      			cSalida = " " ;															//Inicializamos cSalida

				MensajeConectadoaServerpic();

				while(LoraInit() == false); 
				LoRa.receive();
		 		//-------------------------
		 		//Cargamos la configuracion
		 		//-------------------------
				DataConfig aCfg = EpromToConfiguracion ();     							 //Leemos la configuracin de la EEprom
				char USUARIO[1+aCfg.Usuario.length()]; 
				(aCfg.Usuario).toCharArray(USUARIO, 1+1+aCfg.Usuario.length());          //Almacenamos en el array USUARIO el nombre de usuario 
				cDispositivo = USUARIO;													 //Lo asignamos a cDispositivo
				lHomeKit = aCfg.lHomeKit;												 //Recuperamos el flag HomeKit	
				lPush = aCfg.lPush;														 //Recuperamos el flag lPush
				char CLIENTEPUSH[1+aCfg.Push.length()]; 								
				(aCfg.Push).toCharArray(CLIENTEPUSH, 1+1+aCfg.Push.length());            //Almacenamos en el array CLIENTEPUSH el nombre de Cliente push 
				cPush = CLIENTEPUSH;													 //Lo asignamos a cPush
				lEstadisticas = aCfg.lEstadisticas;										 //Recuperamos el flag lEstadisticas
				lTelegram = aCfg.lTelegram;												 //Recuperamos el flag lTelegram
				
				if ( lEstadisticas )													 //Si están habilitadas las estadisticas, actualizamos el numero de inicios
				{
					GrabaVariable ("inicios", 1 + LeeVariable("inicios") );
				}


    			cSalida = LeeValor();													//Recuperamos con el ultimo valor
      			if ( cSalida == "ERROR")												//Si no habia ultimo valor, arrancamos con On
      			{
      				DispositivoOn();
      			}else{																	//Si existia ultimo valor, arrancamos con el valor registrado
      				if (cSalida == "On")
      				{
      					DispositivoOn();
      				}
      				if (cSalida == "Off")
      				{
      					DispositivoOff();
      				}	

      			}	
    		}
    	}	
	}
}


void loop() {

    
    

	/*----------------
	Analisis Lora
 	------------------*/

/*
		String cTexto = TestRecepcionLora();
		if (cTexto.length()>0)
		{
			MensajeTxtRecibidodeLora(cTexto);						//Actualizamos display
			MensajeServidor (cTexto.c_str());						//Mandamos al servidor el mensaje recibido del Lora remoto
		}
*/
		if (oLoraMensaje.lRxMensaje)									//Comprobamos si se ha recibido informacion por radio y si es asi le damos prioridad a la radio
		{
			oLoraMensaje.lRxMensaje = 0;								//Resetasmo el flag de informacion recibida por radio			
	    	#ifdef Debug	
				Serial.println("loop oLoraMensaje.lRxMensaje = 1");			
				Serial.println(oLoraMensaje.Remitente);						//Ejecutamos acciones
				Serial.println(oLoraMensaje.Mensaje);
				Serial.println((oLoraMensaje.Mensaje).length());
			#endif
		}

	    long Inicio, Fin;

 		/*----------------
 		Comprobacion Reset
 		------------------*/

		//TestBtnReset (PinReset);

 		/*-------------------
 		Comprobacion Conexion
 		---------------------*/
		if ( TiempoTest > 0 )
		{
			if ( millis() > ( nMiliSegundosTest + TiempoTest ) )			//Comprobamos si existe conexion  
			{

				nMiliSegundosTest = millis();
				if ( !TestConexion(lEstadisticas) )							//Si se ha perdido la conexion
				{
					lConexionPerdida = 1;									//Ponemos el flag de perdida conexion a 1
					//Acciones que se desean ejecutar cuando 
					//se pierde conexion														
				}else{														//Si existe conexion
					if ( lConexionPerdida )									//Comprobamos si es una reconexion ( por perdida anterior )
					{														//Si lo es
						lConexionPerdida = 0;								//Reseteamos flag de reconexion
						//Acciones que se desean ejecutar cuando 
						//se recupera la conexion																			
					}	
				}	
 	   		}	
    	}
 	
 		/*----------------
 		Analisis comandos
 		------------------*/
		oMensaje = Mensaje ();								 			//Iteractuamos con ServerPic, comprobamos si sigue conectado al servidor y si se ha recibido algun mensaje

		if ( oMensaje.lRxMensaje)										//Si se ha recibido ( oMensaje.lRsMensaje = 1)
		{
	    	#ifdef Debug				
				Serial.println(oMensaje.Remitente);						//Ejecutamos acciones
				Serial.println(oMensaje.Mensaje);
			#endif	

			TelegramaToLora(oMensaje);									//Enviamos el mensaje a Lora remoto


	 		/*----------------
 			Actualizacion ultimo valor
 			------------------*/
			if ( cSalida != String(' ') )								//Si hay cambio de estado
			{	
				EnviaValor (cSalida);									//Actualizamos ultimo valor
			}
	
			cSalida = String(' ');										//Limpiamos cSalida para iniciar un nuevo bucle

			if ( lEstadisticas )									 	//Si están habilitadas las estadisticas, actualizamos el numero de comandos recibidos
			{
				GrabaVariable ("comandos", 1 + LeeVariable("comandos") );
			}	

			nMiliSegundosTest = millis();		
	
		}

	    wdt_reset(); 													//Refrescamos WDT

}
