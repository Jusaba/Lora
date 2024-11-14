# Lora
Se inicia el proyecto con dispositivo Lora junto a ESP32. El código se ha ejecutado satisfactoriamente en los módulos **LoRa 32 V2**

Se han utilizado las siguientes librerias externas 

   	https://github.com/sandeepmistry/arduino-LoRa de Sandeep Mistry
	https://github.com/adafruit/Adafruit-GFX-Library de adafruit
	https://github.com/adafruit/Adafruit_SSD1306 de adafruit
	https://github.com/fbiego/ESP32Time	de fbiego


Para manejar Lora con Serverpic se han editado una serie de funciones recogidas en **LoraServerpic** depositada en el directorio Librerias/LibLora de este repositorio. 

Estas librerias deben instalarse en el path habitual de librerias que suele ser C:\Users\<Usuario>\Documentos\Arduino\libraries. Se incluyen en este repositorio para tenerlas actualilzadas y disponibles.

## Funcionamiento
La tecnología Lora la vamos a emplear alli donde no llega nuestra red de WIfi, se han incluido dos modulos distintos, el **Master** y el **Slave** , 
El **Master** tiene como misión hacer de puente entre Serverpic y los **Slave**. El prinicipio de funcionamienrto es el siguiente:

El **Master** se ubica en un punto donde tenemos Wifi, en el arranque se conecta a Serverpic y básicamente queda a la espera de recibir mensajes de Serverpic y diferenciar si son para el mismo o para algún **Slave**. En caso de que sea para un **Slave**, el **Master** lo que hará será transmitir por radio la orden recibida por Serverpic al **Slave** deseado.

Un mensaje para el **Master** tendrá el formato habitual de Serverpic

	mensaje-:-<Master>-:-<orden>

Un menmsaje para un **Slave** tendrá el siguiente formato en Serverpic

	mensaje-:-<Master>-:-#R-:-<Slave>-:-<orden>

				o por Broadcast

	mensaje-:--<Master>-:-#R-:-broadcast-:-<orden>  	

Si es una orden para un **Slave**, el **Master** elaborará un mensaje similar a los de Serverpic y lo enviará por radio. El mensaje tendra el siguiente formato

	<Remitente>-:-<Slave>-:-<orden>

				o

	<Remitente>-:-broadcast-:-<orden>				


El **Slave** se ha intentado hacer semejante a cualquier dispositivo Serverpic pero teniendo en cuanta que los telegramas ahora los recibirá por radio desde el **Master**. Las rutinas de recepción de la radio estarán en espera y cuando recibe un telegrama pondrá a **1** el flag  **oLoraMensaje.lRxMensaje** y cederá el análisis al loop del programa principal.
En el programa principal, se analiza si el telegrama va dirigido a este **Slave** o si es de broadcast, si es así ejecutará la orden recibida y si no, la ignorará

En la otra dirección, el **Slave**, si tiene que responder a la orden recibida, elaborará un mensaje de la siguiente forma, supongamos que el **Master** recibe el mensaje siguiente desde Serverpic

	mensaje-:-<Master>-:-#R-:-<Slave>-:-<orden>

Tal como se explicó anteriormente, El **Master** manda por radio el siguien te telegrama

	<Remitente>-:-<Slave>-:-<orden>

El **Slave** que coincida con el nombre especificado en el mensaje manda por radio el siguiente texto

	<Remitente>-:-<Slave>-:-<respuesta>

El **Master** recibe ese texto y elabora el siguiente mensaje hacia Serverpic

	mensaje-:-<Remitente>-:-<Slave>-:-<respuesta>

Con esto, el usuario de Serverpic que mandó una orden a un **Salve** recibirá la respuesta a esa orden desde el **Slave** direccionado.	


En el **Master**, la estructura loop del programa principal queda de la siguiente forma

```C++
void loop() {

	/*----------------------------
	* Bloque de codigo auxiliares
	*             .
	*             .
	----------------------------*/

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
	if (oLoraMensaje.lRxMensaje)													//Comprobamos si se ha recibido informacion por radio y si es asi le damos prioridad a la radio
		{
			oLoraMensaje.lRxMensaje = 0;											//Resetasmo el flag de informacion recibida por radio			
			if ( !lErrorRxLora )													//Si se ha recibido sin error.....
			{
				lTxLora = 0;														//Reseteamos el flag de peticion hecha a Esclavo
				nPosMedida = (oLoraMensaje.Mensaje).indexOf("medida-:-");			//Comprobamos si el mensaje recibido es de medida
				if (nPosMedida > 0)													//Si lo es .... Lo mandamos al servidor como mensaje de valor/medida 
				{
					cMensajeMedida = String(oLoraMensaje.Mensaje).substring(  nPosMedida,  String(oLoraMensaje.Mensaje).length() );		
					MensajeServidor(cMensajeMedida);
				}else{																//Si no lo es....
					oMensaje.Mensaje = oLoraMensaje.Mensaje;						//Confeccionamos el mensaje a enviar hacia el servidor		
					oMensaje.Destinatario = oLoraMensaje.Remitente;
					EnviaMensaje(oMensaje);											//Y lo enviamos
				}
			}else{																	//Si la recepcion ha sido erronea ......


			}	

		}
	/*--------------------------------------
	Analisis Comandos recibidos de Serverpic
	----------------------------------------*/
 	oMensaje = Mensaje ();								 					 
 	if ( oMensaje.lRxMensaje)										
 	{
		/*--------------------------------------------------------------------------------------
		* Bloque para la gestión de un comando que se dirige a un remoto ( con el texto '#R-:-')
		---------------------------------------------------------------------------------------*/
		if ((oMensaje.Mensaje).indexOf("#R-:-") == 0)				//Si el inicio del mensaje es #R es mensaje para enviar a los remotos
		{
			oMensaje.Mensaje = String(oMensaje.Mensaje).substring(  3 + String(oMensaje.Mensaje).indexOf("-:-"),  String(oMensaje.Mensaje).length() ); //Excluimos #R del mensaje
			MasterToLora(oMensaje);
		}
		/*--------------------------------
		* Resto de comando  de serverpic
		*             .
		*             .
		---------------------------------*/
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
 	wdt_reset(); 													
}
```

Si se recibe mensaje de Serverpic, le pasa a la función **TelegramaToLora** el contenido del mensaje dentro de una estructura **Telegrama**. La función se encarga de convertir el telegrama en un texto y transmitirlo por la radio.

El Slave recibe ese texto y lo trata segun el siguiente esquema

```C++
void loop() {
    	

  
  if (oLoraMensaje.lRxMensaje)	
  {
		oLoraMensaje.lRxMensaje = 0;
		String cOrdenLora = String(oLoraMensaje.Mensaje).substring(  3 + String(oLoraMensaje.Mensaje).indexOf("-:-"),  String(oLoraMensaje.Mensaje).length() ); //Extraemos los parametros
		String cDestinatarioLora = String(oLoraMensaje.Mensaje).substring(0, String(oLoraMensaje.Mensaje).indexOf("-:-"));
		if ( cDestinatarioLora == cDispositivo || cDestinatarioLora =="broadcast" )
		{
			if ( cDestinatarioLora =="broadcast" )
			{ 
				lBroadcast = 1;										//Ponemos a 1 el flag de broadcast. Si es broadcast no debe haber respuesta del Slave
			}

     			//*******************************************************************************
     			//En este punto empieza el bloque de programa particular del dispositivo según la 
     			//utilización de ordenes recibidas por radio
     			//Se analiza el mensaje recibido por radio y se actúa en consecuencia
     			//*******************************************************************************	
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
  oMensaje = Mensaje ();		 					 
  if ( oMensaje.lRxMensaje)										
  {
	 oMensaje.lRxMensaje = 0;
     //*******************************************************************************
     //En este punto empieza el bloque de programa particular del dispositivo según la 
     //utilización de ordenes recibidas por Serverpic
     //Se analiza el mensaje recibido desde Serverpic y se actúa en consecuencia
     //*******************************************************************************
  }
  wdt_reset(); 													
}
```
La radio esta en recepción permanente, si recibe un texto, la función **onRecieve** de **LoraServerpic.h** se encarga de obtener el mensaje y convertirlo a la variable **oLoraMensaje** con formato **Telegrama** con el flag **lRxMensaje** a 1,  analizaliza el Telegrama **oLoraMensaje**  de forma que el bucle de analisis de comandos recibidos por radio es similar al bucle utilizado para Serverpic.

En la otra dirección, si el **Slave** debe contestar, elabora un String con el formato

	<Remitente>-:-<Slave>-:-<Respuesta>

Con ese String, la función  **StringToLora()** transmite la cadena por radio. La rutina **Onreceive()** del **Master** lo recibe y pone el flag **lRxMensaje** a 1, en el loop, la parte de codigo que se encarga de atender la recepción de la radio elabora un mensaje de Serverpic y lo envia según el siguiente código

```C++
		if (oLoraMensaje.lRxMensaje)								//Comprobamos si se ha recibido informacion por radio y si es asi le damos prioridad a la radio
		{
			oLoraMensaje.lRxMensaje = 0;							//Resetasmo el flag de informacion recibida por radio			
			oMensaje.Mensaje = oLoraMensaje.Mensaje;				//Confeccionamos el mensaje a enviar hacia el servidor	
			oMensaje.Destinatario = oLoraMensaje.Remitente;
			EnviaMensaje(oMensaje);									//Y lo enviamos
		}
```		

En el momento en el que se elabora esta documentación, se han contemplado dos Slaves distintos, Salave_Rele y Slave_TUF2000M

## Salave_Rele

Este módulo se ha diseñado para controlar un relé. Es muy básico y tiene los siguientes comandos/mensajes

```	
			On.- Pone el relé en On
			On-:-N.- Pone el relé  On durante N minutos
			Off.- Pone el relé en Off
			Get.- Devuelve el estado del relé
			fecha-:-DD-:-MM-:-YYYY-:-HH-:-MM-:-SS.- Actualiza el RTC con los datos transferidos
			Reset.- Resetea el modulo
```

## Salave_TUF2000M

Este modulo se ha diseñado para obtener los datos de consumo de agua utilizando  medidor de caudal ultrasonico TUF-2000M
Se ha diseñado la libreria **TUF2000M.h** para gestionar el medidor de caudal.

Los comandos disponibles para este Slave son:

```
	fecha-:-DD-:-MM-:-YYYY-:-HH-:-MM-:-SS.- Actualiza el RTC con los datos transferidos
	Configura.- Configura el Tuf-2000M
	ConsumoAcumulado.- Devuelve el consumo acumulado almacenado en el registro  'positive acumulator' (0009) 
	Reset.- Resetea el modulo

```

