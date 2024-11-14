/**
******************************************************
* @file Master.ino
* @brief Master de sistema con radio Lora
* @author Julian Salas Bartolome
* @version 1.0
* @date 01/10/2024
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
	EEPROM.begin(256);																	//Reservamos zona de EEPROM
	//BorraDatosEprom ( 0, 256 );														//Borramos 128 bytes empezando en la posicion 0		

  	//Declaracion de Entradas/Salidas
	//pinMode(PinReset, INPUT_PULLUP);           											//Configuramos el pin de reset como entrada 
	#ifdef Display
		MensajeInicio();
	#endif

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
				#ifdef Display
					MensajeConectadoaServerpic();
				#endif
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

				lInicio = 1;															//Ponemos el flag lInicio a 1 para informar al loop que se acaba de iniciar el modulo

    			cSalida = LeeValor();													//Recuperamos con el ultimo valor
      			if ( cSalida == "ERROR")												//Si no habia ultimo valor, arrancamos con On
      			{
       			}else{																	//Si existia ultimo valor, arrancamos con el valor registrado

	    		}
    		}	

		}
	}	
}


void loop() {


    //--------------------------------------------------------------------------
	//Bloque que se ejecuta cada segundo
	//Utilizado para refrescar el reloj de la pantalla si es que hay pantalla
	//-------------------------------------------------------------------------
		nSegundosCiclo = rtc.getSecond();
		if ( nSegundosTime != nSegundosCiclo )
		{
			if ( nSegundosCiclo < nSegundosTime )	
			{
				nSegundosCicloDif = ( 60 - nSegundosTime) + nSegundosCiclo;
			}else{
				nSegundosCicloDif = nSegundosCiclo - nSegundosTime;
			}
			nSegundosTime = nSegundosCiclo;
			#ifdef Display	
				LimpiaPantalla();				
				MensajeDispositivo (cDispositivo);
				MensajeHora (rtc.getSecond(), rtc.getMinute(), rtc.getHour(true));
				VisualizaPantalla();
			#endif
		}

    //--------------------------------------------------------------------------
	//Bloque que se ejecuta cada cambio de minuto
	//Se utiliza para pedir información a escalvos
	//En este caso, se utiliza para solicitar el caudal acumulado a un TUF-2000M
	//-------------------------------------------------------------------------
		nMinutosCiclo = rtc.getMinute();
		if ( nMinutosTime != nMinutosCiclo )
		{
			nMinutosTime = nMinutosCiclo;
			#ifdef TUF2000M	
				oMensaje.Remitente = cDispositivo;					//Preparamos mensje para lora_2 
				oMensaje.Mensaje = "lora_2-:-ConsumoAcumulado" ;
				MasterToLora (oMensaje);
			#endif	

		}
    //--------------------------------------------------------------------------
	//Bloque que se ejecuta para repetir una solicitud a un escalvo
	//Cuando el Maestro solicita información a un esclavo, el flag lTxLora se pone a 1 para indicar a loop que se ha realizado peticion y si al cabo de
	//'SgparaRepeticion' segundos no se ha recibido respuesta, el Master repite la solicitud 
	//Este proceso ses repite un numero 'RepeticionesTx' de veces
	//-------------------------------------------------------------------------
		if (( millis() > (nMilisegundosRepeticion + (1000 * SgparaRepeticion))) & lTxLora & nRepeticiones > 0)
		{
			MasterRepiteToLora ();
		}
	/*-------------------------------------------------------------------------
	Analisis Lora
 	Si se recibe un mensaje por Radio ( oLoraMensaje.lRxMensaje = 1 ), reseteamos el flag oLoraMensaje.lRxMensaje
	Comprobamos si se ha recibido sin error con el estado del flag lErrorRxLora. Si se ha recibido sin error, reseteamos 
	el flag lTxLora para alertar de que se ha recibido la información solicitada. Luego se comprueba si la petición 
	fue para obtener una medida o de información.
	Si fue de medida se prepara mensaje para servidor con el valor de la medida para que el servidor la mande a slastic.
	La estructura es

		medida-:-<Parametro>-:-<Valor> 

	Si se trataba de una solicitud de información se prepara un mensaje de respuesta al peticionario con la sigiente estructura

		oLoraMensaje.Remitente = El usuario de Serverpic que solicito una accion de un remoto de Lora
		oLoraMensaje.Mensaje = <Nombre remoto Lora>-:-<Acción realizada>
	--------------------------------------------------------------------------*/
		if (oLoraMensaje.lRxMensaje)																									//Comprobamos si se ha recibido informacion por radio y si es asi le damos prioridad a la radio
		{
			oLoraMensaje.lRxMensaje = 0;																								//Resetasmo el flag de informacion recibida por radio			
	    	#ifdef Debug	
				Serial.println("loop oLoraMensaje.lRxMensaje = 1");			
				Serial.println(oLoraMensaje.Remitente);																		
				Serial.println(oLoraMensaje.Mensaje);
				Serial.println((oLoraMensaje.Mensaje).length());
				Serial.println ( lErrorRxLora ? "Recepcion con error" : "Recepcion sin error");
			#endif
			if ( !lErrorRxLora )																										//Si se ha recibido sin error
			{
				lTxLora = 0;																											//Reseteamos el flag de peticion hecha a Esclavo
				nPosMedida = (oLoraMensaje.Mensaje).indexOf("medida-:-");																//Comprobamos si el mensaje recibido es de medida
				if (nPosMedida > 0)																										//Si lo es ....
				{
					cMensajeMedida = String(oLoraMensaje.Mensaje).substring(  nPosMedida,  String(oLoraMensaje.Mensaje).length() );		//Mandamso a servidor mensaje de medida
					MensajeServidor(cMensajeMedida);
				}else{																													//Si no lo es....
					oMensaje.Mensaje = oLoraMensaje.Mensaje;																			//Confeccionamos el mensaje a enviar hacia el servidor		
					oMensaje.Destinatario = oLoraMensaje.Remitente;
					EnviaMensaje(oMensaje);																								//Y lo enviamos
				}
			}else{
																																		//Si la recepcion ha sido erronea
																																		//
			}	

		}

	 
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

 	/*---------------------
 	Comprobacion de si es el primer ciclo de loop desde que ha arrancado el dispositivo
 	---------------------*/
		if (lInicio)														//Si ha arrancado y se ha conectado al servidor
		{
			lInicio = 0;													//Reseteamos el flag lInicio para que no se repta este proceso
			MensajeServidor ("fecha-:-"+cDispositivo);						//Solicitamos la hora al servidor para actualizar RTC y enviarsela a los esclavos
		}

	/*--------------------------------------
	Analisis Comandos recibidos de Serverpic
	----------------------------------------*/		
	oMensaje = Mensaje ();								 			//Iteractuamos con ServerPic, comprobamos si sigue conectado al servidor y si se ha recibido algun mensaje

		if ( oMensaje.lRxMensaje)										//Si se ha recibido ( oMensaje.lRsMensaje = 1)
		{
	    	#ifdef Debug				
				Serial.println(oMensaje.Remitente);						//Ejecutamos acciones
				Serial.println(oMensaje.Mensaje);
			#endif	
		//Si el mensaje empieza con '#R-:-' es un mensaje dirigido a un remoto
		// El formato para los remotos es 'mensaje-:-<Maestro>-:-#R-:-<Nombre remoto>-:-Orden 
			if ((oMensaje.Mensaje).indexOf("#R-:-") == 0)				//Si el inicio del mensaje es #R es mensaje para enviar a los remotos
			{
				oMensaje.Mensaje = String(oMensaje.Mensaje).substring(  3 + String(oMensaje.Mensaje).indexOf("-:-"),  String(oMensaje.Mensaje).length() ); //Extraemos el mensaje excluyendo el #R
				MasterToLora(oMensaje);																														//Enviamos el mensaje a Lora remoto habilitando la repeticion sui fuera necesario
			}	 
			if ((oMensaje.Mensaje).indexOf("fecha-:-") == 0)			//Si se recibe 'fecha'
			{
				String cMensaje =  String(oMensaje.Mensaje).substring(  3 + String(oMensaje.Mensaje).indexOf("-:-"),  String(oMensaje.Mensaje).length() );
				String cDia = cMensaje.substring (0, String(cMensaje).indexOf("-:-") );
				cMensaje =  String(cMensaje).substring(  3 + String(cMensaje).indexOf("-:-"),  String(cMensaje).length() );
				String cMes = cMensaje.substring (0, String(cMensaje).indexOf("-:-") );
				cMensaje =  String(cMensaje).substring(  3 + String(cMensaje).indexOf("-:-"),  String(cMensaje).length() );
				String cAno = cMensaje.substring (0, String(cMensaje).indexOf("-:-") );
				cMensaje =  String(cMensaje).substring(  3 + String(cMensaje).indexOf("-:-"),  String(cMensaje).length() );
				String cHora = cMensaje.substring (0, String(cMensaje).indexOf("-:-") );
				cMensaje =  String(cMensaje).substring(  3 + String(cMensaje).indexOf("-:-"),  String(cMensaje).length() );
				String cMinutos = cMensaje.substring (0, String(cMensaje).indexOf("-:-") );
				String cSegundos = String(cMensaje).substring(  3 + String(cMensaje).indexOf("-:-"),  String(cMensaje).length() );
				SetHora (cSegundos.toInt(), cMinutos.toInt(), cHora.toInt());
				SetFecha (cDia.toInt(), cMes.toInt(), cAno.toInt());
				oMensaje.Remitente = cDispositivo;					//Preparamos mensje para broadcast, remitente el <Maestro> y mensaje <Maestro>All-:-fecha-.-..... 
				oMensaje.Mensaje = "broadcast-:-fecha-:-"+String(oMensaje.Mensaje).substring(  3 + String(oMensaje.Mensaje).indexOf("-:-"),  String(oMensaje.Mensaje).length() );
				TelegramaToLora(oMensaje);							//	Enviamos mensaje sin habilitar la recepción, a un broadcast los Eslave no deben responder nada																		
			}					

	 	/*----------------
 		Actualizacion ultimo valor
 		------------------*/
			if ( cSalida != String(' ') )								//Si hay cambio de estado
			{	
				EnviaValor (cSalida);									//Actualizamos ultimo valor
			}
	
			cSalida = String(' ');										//Limpiamos cSalida para iniciar un nuevo bucle
	 	/*----------------
 		Actualizacion las estadisticas
 		------------------*/
			if ( lEstadisticas )									 	//Si están habilitadas las estadisticas, actualizamos el numero de comandos recibidos
			{
				GrabaVariable ("comandos", 1 + LeeVariable("comandos") );
			}	

			nMiliSegundosTest = millis();		
	
		}

	    wdt_reset(); 													//Refrescamos WDT

}
