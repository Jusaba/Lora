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
	cDispositivoMaster = String(cDispositivo).substring(  0, String(cDispositivo).indexOf("_"));
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

				lInicio = 1;															//Flag que indica si se ha conectado a Serverpic

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

	//Controlamos un On Temporizado
	nSegundosCiclo = rtc.getSecond();											//Actualizamos la variable nSegundoCiclo con los segundos de RTC
	if ( nSegundosTime != nSegundosCiclo )										//Si se ha cambiado de segundo en l RTC
	{
		if ( nSegundosCiclo < nSegundosTime )									//Miramos si RTC ha cambiado de minuto para calcular los segundos reales transcurridos		
		{																		// y almacenamos los segundos transcurridos en nSegundosCicloDif
			nSegundosCicloDif = ( 60 - nSegundosTime) + nSegundosCiclo;			
		}else{
			nSegundosCicloDif = nSegundosCiclo - nSegundosTime;
		}
		nSegundosTime = nSegundosCiclo;											//Actualiamos nSegundosTime con el segundo actual para la siguiente comprobacion
		LimpiaPantalla();														//Borramos la pantalla
		if ( !lTemporizado )													//Si no hay proceso temporizado
		{
			if ( lEstado )															//Si es estado On
			{	
				MensajeOn();															//Preparamos el mensaje On para la pantalla											
			}else{																		//Si el Estado es Off
				MensajeDispositivo (cDispositivo);										//Preparamos el mensaje de Dispositibo + Hora ( estado en espera )
				//MensajeOff();
				MensajeHora (rtc.getSecond(), rtc.getMinute(), rtc.getHour(true));
			}
		}else{																		//Si es estado Off						
			SegundosToHHMMSS (nSegundosOn);											//Visualizamos en pantalla el tiempo restante del temporizado en HH:MM:SS
			nSegundosOn = nSegundosOn - nSegundosCicloDif;							//Actualizamos los segundos que quedan de temporizacion
			if ( nSegundosOn <  1)													//Si se ha llegado al final de la temporizacion
			{
				lTemporizado = 0;													//Ponemos el flag de temporizacion a 0
				LimpiaPantalla();													//Limpiamos la pantalla
				DispositivoOff();													//Ponemos el dispositivo en Off
				MensajeDispositivo (cDispositivo);									//y dejamos en pantalla el mensaje de en espera
				MensajeHora (rtc.getSecond(), rtc.getMinute(), rtc.getHour(true));
			}
		}	
		VisualizaPantalla();														//Visualizamos el mensaje elaborado
	}	

	/*----------------
	Analisis Lora
 	Si se recibe un mensaje por Radio ( oLoraMensaje.lRxMensaje = 1 ), reseteamos el flag oLoraMensaje.lRxMensaje
	Confeccionamos el mensaje hacia Serverpic y lo enviamos a Serverpic
		oLoraMensaje.Remitente = El usuario de Serverpic que solicito una accion de un remoto de Lora
		oLoraMensaje.Mensaje = <Nombre remoto Lora>-:-<Acción realizada>

		Mensajes validos:
			On.- Pone el dispositivo en On
			On-:-N.- Pone el dispositivo en On durante N minutos
			Off.- Pone el dispositivo en Off
			Get.- Devuelve el estado del dispositivo
			fecha-:-DD-:-MM-:-YYYY-:-HH-:-MM-:-SS.- Actualiza el RTC con los datos transferidos
			Reset.- Resetea el modulo
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
		if ( cDestinatarioLora == cDispositivo || cDestinatarioLora =="broadcast" )
		{
			if ( cDestinatarioLora =="broadcast" )
			{ 
				lBroadcast = 1;										//Ponemos a 1 el flag de broadcast. Si es broadcast no debe haber respuesta del Slave
			}
			if (cOrdenLora.indexOf("On") == 0)						//Si se recibe "On"
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
			if (cOrdenLora == "Off")								//Si se recibe "Off"
			{	
				if (lTemporizado )
				{
					lTemporizado = 0;
				}
				DispositivoOff();	
				cSalida = "Off";
			}	
			if (cOrdenLora == "Get")								//Si se recibe "Get"
			{	
				if ( GetDispositivo() )
				{
					cSalida = "On";
				}else{
					cSalida = "Off";
				}	
				
			}				
			if (cOrdenLora.indexOf("fecha-:-") == 0)				//Si se recibe "Fecha"
			{
				String cMensaje =  String(cOrdenLora).substring(  3 + String(cOrdenLora).indexOf("-:-"),  String(cOrdenLora).length() );
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
			}		
			if (cOrdenLora == "Reset")								//Si se recibe "Reset"
			{	
				Reset();				
			}									
			if (cOrdenLora == "Master")								//Si se recibe "Master"
			{	
				nMiliSegundosTest = millis();				
			}							
		}
		#ifdef Display			
			MensajeTxtRecibidodeLora(oLoraMensaje);
		#endif	
		/*----------------
 		Contestacion al Master
 		------------------*/
		if ( cSalida != String(' ') && !lBroadcast )									//Si hay algo que comunicar y la orden no fue a Broadcast
		{	
			StringToLora (oLoraMensaje.Remitente+"-:-"+cDispositivo+"-:-"+cSalida);		//Se le notifica al Lora Remitente
			#ifdef Display
				//TextoEnviadoaLora (cDispositivo+"-:-"+cSalida);
				MensajeTxtEnviadoaLora (oLoraMensaje);
			#endif 
		}	
		cSalida = String(' ');	
		lBroadcast = 0;


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
			if ( millis() > ( nMiliSegundosTest + TiempoTest ) )			//Comprobamos si se ha perdiod la conexion con el Master 
			{

				//Reset();
				nMiliSegundosTest = millis();
				Serial.print("#");
 	   		}	
    	}

		if (lInicio)														//Si ha arrancado y se ha conectado al servidor
		{
			lInicio = 0;													//Reseteamos el flag lInicio para que no se repta este proceso
			MensajeServidor ("fecha-:-"+cDispositivo);						//Solicitamos la hora al servidor
		}

 		/*----------------
 		Analisis comandos

		Comandos validos

			On.- Pone el dispositivo en On
			Off.- Pone el dispositivo en Off
			Get.- Devuelve el estado del dispositivo
			Change.- El disisitivo cambia de estado
			ChangeGet.- El disisitivo cambia de estado y devuelve el nuevo estado
			fecha-:-DD-:-MM-:-YYYY-:-HH-:-MM-:-SS.- Actualiza el RTC con los datos transferidos

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
			if ((oMensaje.Mensaje).indexOf("fecha-:-") == 0)								//Si se recibe 'Hora'
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
			}		
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
