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



//SoftwareSerial swSerial(RX_PIN, TX_PIN);

 


void setup() {


  	#ifdef Debug																		//Usamos el puereto serie solo para debugar	
		Serial.begin(9600);																//Si no debugamos quedan libres los pines Tx, Rx para set urilizados
		Serial.println("Iniciando........");
		
	#endif
	EEPROM.begin(256);																	//Reservamos zona de EEPROM

  	Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  	TUF.begin(MODBUS_DEVICE_ID, Serial2);


//  ConfiguraIdioma();
//	ConfiguraUnidades();
//	ConfiguraPipe();


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

	}
	Serial.print("Setup: ");
	Serial.println (nthickness);		

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
		#ifdef Display	
			LimpiaPantalla();													//Borramos la pantalla
			MensajeDispositivo (cDispositivo);									//Preparamos el mensaje de Dispositibo 
			MensajeHora (rtc.getSecond(), rtc.getMinute(), rtc.getHour(true));  //Visualizamos la hora
			VisualizaPantalla();		
		#endif	

	}	

	/*----------------
	Analisis Lora
 	Si se recibe un mensaje por Radio ( oLoraMensaje.lRxMensaje = 1 ), reseteamos el flag oLoraMensaje.lRxMensaje
	Confeccionamos el mensaje hacia Serverpic y lo enviamos a Serverpic
		oLoraMensaje.Remitente = El usuario de Serverpic que solicito una accion de un remoto de Lora
		oLoraMensaje.Mensaje = <Nombre remoto Lora>-:-<Acción realizada>

		Mensajes validos:
			fecha-:-DD-:-MM-:-YYYY-:-HH-:-MM-:-SS.- Actualiza el RTC con los datos transferidos
			Configura.- Configura el Tuf-2000M
			ConsumoAcumulado.- Devuelve el consumo acumulado almacenado en el registro  'positive acumulator' (0009) 
			Calidad.- Devuelve una cadena con los siguientes datos Upstream strength-:-Downstream strength-:-Signal Quality
			Error.- Devuelve un entero con el codigo de error. Los códigos de error se describen en la Nota 4 del manual
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
			Serial.println ( lErrorRxLora ? "Recepcion con error" : "Recepcion sin error");
		#endif	
		if ( !lErrorRxLora )																	//Si se ha recibido mensaje de radio sin error....
		{		
			if ( cDestinatarioLora == cDispositivo || cDestinatarioLora =="broadcast" )			//Si el mensaje va dirigido a este slave o es un broadcast
			{
				if ( cDestinatarioLora =="broadcast" )											//Si es broadcast ponemos a 1 el flag lBroadcast
				{ 
					lBroadcast = 1;																//Ponemos a 1 el flag de broadcast. Si es broadcast no debe haber respuesta del Slave
				}				
				if (cOrdenLora.indexOf("fecha-:-") == 0)										//Si se recibe "Fecha"
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
					ConfiguraHoraFecha (cSegundos.toInt(), cMinutos.toInt(), cHora.toInt(), cDia.toInt(), cMes.toInt(), cAno.toInt());
					StringToLora ("Master-:-Ok");												//Respondemos al Master
				}		
				if (cOrdenLora == "Reset")														//Si se recibe "Reset"
				{	
					StringToLora (oLoraMensaje.Remitente+"-:-"+cDispositivo+"-:-Reseteado");	//Se manda por radio para que recoja el master y pueda responder al remitente y Master no repita	
					delay(500);				
					Reset();				
				}									
				if (cOrdenLora == "Configura")													//Si se recibe "Configura"
				{	
					Configura();	
					cSalida = "TUF-2000M Configurado";			
				}							
				if (cOrdenLora == "ConsumoAcumulado")											//Si se recibe "ConsumoAcumulado"...
				{	
					AcumuladoAgua = ReadPositiveAcumulator();										//Obtenemos el consumo acumulado
					cSalida = ("medida-:-Jardin-:-"+(String)(AcumuladoAgua));						//Lo enviamos al Master como Medida
				}	
				if (cOrdenLora == "Calidad")													//Si se recibe Calidad
				{
					cSalida = (String)(ReadUStrength());
					delay(100);
					cSalida = cSalida + "-:-" + (String)(ReadDStrength());
					delay(100);
					cSalida = cSalida + "-:-" + (String)(ReadQ());
				}
				if (cOrdenLora == "Error")														//Si se recibe error
				{
					cSalida = (String)(ReadErrorCode());
				}				
				if (cOrdenLora == "Master")														//Si se recibe "Master"
				{	
					nMiliSegundosTest = millis();												//Reseteamos el contador de Test para evitar reset
				}							
			}
			/*----------------
 			Contestacion al Master
 			------------------*/
			if ( cSalida != String(' ') && !lBroadcast )									//Si hay algo que comunicar y la orden no fue a Broadcast
			{	
				StringToLora (oLoraMensaje.Remitente+"-:-"+cDispositivo+"-:-"+cSalida);		//Se manda por radio para que recoja el master y pueda responder al remitente
			}	
			cSalida = String(' ');	
			lBroadcast = 0;
		}else{																					//Si ha habido error....
																								//No respondemos y forzamos a una repeticion de peticion al Master	
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
		oMensaje = Mensaje ();							 				//Iteractuamos con ServerPic, comprobamos si sigue conectado al servidor y si se ha recibido algun mensaje

		if ( oMensaje.lRxMensaje)										//Si se ha recibido ( oMensaje.lRsMensaje = 1)
		{
			oMensaje.lRxMensaje = 0;


	 		/*----------------
 			Actualizacion ultimo valor
 			------------------*/
			if ( cSalida != String(' ') )								//Si hay cambio de estado
			{	
				EnviaValor (cSalida);									//Actualizamos ultimo valor
			}
	
			cSalida = String(' ');										//Limpiamos cSalida para iniciar un nuevo bucle

			nMiliSegundosTest = millis();		
	
		}
	    wdt_reset(); 													//Refrescamos WDT
//		readFlow();
//		delay(100);
//		ReadPositiveAcumulator();
		//LeeRegistrosLong (103 );
		//delay(100);
		//FlowForTodayDecimal();
		//delay(100);
		//FlowForMonthDecimal();
		//delay(100);
		//FlowForYearDecimal();
		//delay(100);
	
	
	//	Serial.print ("Q: ");
	//	Serial.println(ReadQ());
	//	delay(100);
	//	Serial.print ("UStrength: ");
	//	Serial.println(ReadUStrength());
	//	delay(100);
	//	Serial.print ("DStrength: ");
	//	Serial.println(ReadDStrength());

	/*
	Serial.println("--------------------");	
	Serial.println(ReadErrorCode());
	delay(100);
	Serial.println("--------------------");	
	*/

	/*	
	for ( int x = 90; x<95; x++)
	{
		Serial.print("------>");
		Serial.println(x);
		LeeRegistrosInt(x);
		delay(1000);
	}
	*/
		delay(1000);

		//delay(1000);
}
