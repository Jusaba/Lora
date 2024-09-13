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


    
  	#ifdef Debug																		//Usamos el puereto serie solo para debugar	
		Serial.begin(9600);																//Si no debugamos quedan libres los pines Tx, Rx para set urilizados
		Serial.println("Iniciando........");
	#endif
	EEPROM.begin(256);																	//Reservamos zona de EEPROM

	Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);												//Inicializamos 1Wire
    DisplayInit();																		//Inicializamos la pantalla

	//rtc.setTime ( 0, 59, 23, 12, 9, 2024 );

	SetHora(0, 59, 23);
	SetFecha ( 12, 9, 2024 );
	//SetHora(0, 59, 23);
	//SetFecha ( 12, 9, 2024 );
	//SetHoraFecha ( 0, 59, 23, 12, 9, 2024 );

	//BorraDatosEprom ( 0, 256 );														//Borramos 128 bytes empezando en la posicion 0		

  	//Declaracion de Entradas/Salidas
	//pinMode(PinReset, INPUT_PULLUP);           											//Configuramos el pin de reset como entrada 

	//#ifdef Display	
		MensajeInicio();
	//#endif

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
				
				if ( lEstadisticas )													 //Si están habilitadas las estadisticas, actualizamos el numero de inicios
				{
					GrabaVariable ("inicios", 1 + LeeVariable("inicios") );
				}


    			cSalida = LeeValor();													//Recuperamos con el ultimo valor
      			if ( cSalida == "ERROR")												//Si no habia ultimo valor, arrancamos con On
      			{
      			}else{																	//Si existia ultimo valor, arrancamos con el valor registrado
      				if (cSalida == "On")
      				{
      				}
      				if (cSalida == "Off")
      				{
      				}	
      			}	
    		}
    	}	
	}
}


void loop() {

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
		LimpiaPantalla();		
		if ( !lTemporizado )
		{
			if ( lEstado )
			{
				MensajeOn();
			}else{
				MensajeDispositivo (cDispositivo);
				//MensajeOff();
				MensajeHora (rtc.getSecond(), rtc.getMinute(), rtc.getHour(true));
			}
		}else{
			SegundosToHHMMSS (nSegundosOn);
			nSegundosOn = nSegundosOn - nSegundosCicloDif;
			if ( nSegundosOn <  1)
			{
				lTemporizado = 0;
				LimpiaPantalla();
				MensajeDispositivo (cDispositivo);
				DispositivoOff();
				MensajeHora (rtc.getSecond(), rtc.getMinute(), rtc.getHour(true));
			}
		}	
		VisualizaPantalla();
	}	

	/*----------------
	Analisis Lora
 	Si se recibe un mensaje por Radio ( oLoraMensaje.lRxMensaje = 1 ), reseteamos el flag oLoraMensaje.lRxMensaje
	Confeccionamos el mensaje hacia Serverpic y lo enviamos a Serverpic
		oLoraMensaje.Remitente = El usuario de Serverpic que solicito una accion de un remoto de Lora
		oLoraMensaje.Mensaje = <Nombre remoto Lora>-:-<Acción realizada>
	------------------*/
	if (oLoraMensaje.lRxMensaje)									//Comprobamos si se ha recibido informacion por radio y si es asi le damos prioridad a la radio
	{
		oLoraMensaje.lRxMensaje = 0;								//Resetasmo el flag de informacion recibida por radio
		String cOrdenLora = String(oLoraMensaje.Mensaje).substring(  3 + String(oLoraMensaje.Mensaje).indexOf("-:-"),  String(oLoraMensaje.Mensaje).length() ); //Extraemos los parametros
		String cDestinatarioLora = String(oLoraMensaje.Mensaje).substring(0, String(oLoraMensaje.Mensaje).indexOf("-:-"));
		#ifdef Debug	
			Serial.println("loop oLoraMensaje.lRxMensaje = 1");			
			Serial.println(oLoraMensaje.Remitente);						//Ejecutamos acciones
			Serial.println(oLoraMensaje.Mensaje);
			Serial.println((oLoraMensaje.Mensaje).length());
			Serial.println(cDestinatarioLora);
			Serial.println(cOrdenLora);
		#endif		
		if ( cDestinatarioLora == cDispositivo )
		{
			if (cOrdenLora.indexOf("On") == 0)									//Si se recibe "On", se habilita el pir
			{	

				if (cOrdenLora.indexOf("On-:-") == 0)				//Si hay parametro de duracion de minutos
				{
					String cMinutos =  String(cOrdenLora).substring(  3 + String(cOrdenLora).indexOf("-:-"),  String(cOrdenLora).length() );
					nSegundosOn = cMinutos.toInt() * 60;
					DispositivoOn();
					lTemporizado = 1;
				}else{
					DispositivoOn();
				}					
				cSalida = "On";
			}
			if (cOrdenLora == "Off")								//Si se recibe "Off", se habilita el pir
			{	
				if (lTemporizado )
				{
					lTemporizado = 0;
				}
				DispositivoOff();	
				cSalida = "Off";
			}	
			if (cOrdenLora == "Get")								//Si se recibe "Get", se habilita el pir
			{	
				if ( GetDispositivo() )
				{
					cSalida = "On";
				}else{
					cSalida = "Off";
				}	
				
			}				
			if (cOrdenLora.indexOf("Hora-:-") == 0)								//Si se recibe 'Hora'
			{
				String cMensaje =  String(cOrdenLora).substring(  3 + String(cOrdenLora).indexOf("-:-"),  String(cOrdenLora).length() );
				String cHora = cMensaje.substring (0, String(cMensaje).indexOf("-:-") );
				cMensaje =  String(cMensaje).substring(  3 + String(cMensaje).indexOf("-:-"),  String(cMensaje).length() );
				String cMinutos = cMensaje.substring (0, String(cMensaje).indexOf("-:-") );
				String cSegundos = String(cMensaje).substring(  3 + String(cMensaje).indexOf("-:-"),  String(cMensaje).length() );
				SetHora (cSegundos.toInt(), cMinutos.toInt(), cHora.toInt());
			}							
		}
		#ifdef Display			
			MensajeTxtRecibidodeLora(oLoraMensaje);
		#endif	
		/*----------------
 		Contestacion al Master
 		------------------*/
		if ( cSalida != String(' ') )								//Si hay cambio de estado
		{	
			StringToLora (oLoraMensaje.Remitente+"-:-"+cDispositivo+"-:-"+cSalida);
			#ifdef Display
				//TextoEnviadoaLora (cDispositivo+"-:-"+cSalida);
				MensajeTxtEnviadoaLora (oLoraMensaje);
			#endif 
		}	
		cSalida = String(' ');	


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
 	
 		/*----------------
 		Analisis comandos
 		------------------*/
		//Serial.println(oMensaje.lRxMensaje);

		oMensaje = Mensaje ();							 				//Iteractuamos con ServerPic, comprobamos si sigue conectado al servidor y si se ha recibido algun mensaje

		if ( oMensaje.lRxMensaje)										//Si se ha recibido ( oMensaje.lRsMensaje = 1)
		{
			oMensaje.lRxMensaje = 0;

			//En este punto empieza el bloque de programa particular del dispositivo segun la utilizacion					

			if (oMensaje.Mensaje == "On")								//Si se recibe "On", se habilita el pir
			{	
				DispositivoOn();	
				cSalida = "On";
			}
			if (oMensaje.Mensaje == "Off")								//Si se recibe "Off", de deshabilita el pri
			{	
				DispositivoOff();	
				cSalida = "Off";
			}
			if (oMensaje.Mensaje == "Change")							//Si se recibe 'Change', cambia el estado del pir
			{	
				if ( GetDispositivo() )
				{
					DispositivoOff();
					cSalida = "Off";
				}else{
					DispositivoOn();
					cSalida = "On";
				}
			}
			if (oMensaje.Mensaje == "ChangeGet")						//Si se recibe 'ChangeGet', cambia el estado del Pir y devuelve el nuevo estado al remitente 
			{	
				if ( GetDispositivo() )
				{
					DispositivoOff();
					cSalida = "Off";
				}else{
					DispositivoOn();
					cSalida = "On";					
				}
				oMensaje.Mensaje = cSalida;								//Confeccionamos el mensaje a enviar hacia el servidor	
				oMensaje.Destinatario = oMensaje.Remitente;
				EnviaMensaje(oMensaje);									//Y lo enviamos
			}			
			if (oMensaje.Mensaje == "Get")								//Si se recibe 'Get', se devuelve el estado del pir al remitente
			{	
				if ( GetDispositivo() )
				{
					cSalida = "On";
				}else{
					cSalida = "Off";
				}
				oMensaje.Mensaje = cSalida;
				oMensaje.Destinatario = oMensaje.Remitente;
				EnviaMensaje(oMensaje);	
				cSalida = String(' ');									//No ha habido cambio de estado, Vaciamos cSalida para que no se envie a WebSocket y a HomeKit 
			}	

	 		/*----------------
 			Actualizacion ultimo valor
 			------------------*/
			if ( cSalida != String(' ') )								//Si hay cambio de estado
			{	
				//EnviaValor (cSalida);									//Actualizamos ultimo valor
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
