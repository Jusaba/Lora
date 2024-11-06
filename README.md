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

El **Slave** tiene la posibilidad de conectarse a Serverpic si está en una zona con Wifi


En el **Master**, la estructura loop del programa principal queda de la siguiente forma

```C++
void loop() {

  TestBtnReset (PinReset);

  	if ( TiempoTest > 0 )
  	{
		if ( millis() > ( nMiliSegundosTest + TiempoTest ) )	
		{
			nMiliSegundosTest = millis();
			if ( !TestConexion() )
			{

  	    	   //**********************************************************************
			   //Aqui pondremos las acciones a ejecutar si se pierde la conexion
			   //Por ejemplo, se puede poner el dispositivo controlado en off que seria
			   //el estado deseado cuando perdemos la conexion y no lo podemos 
			   //telecontrolar
  	    	   //**********************************************************************
			}												                                                         
 		}	
  	}
	/*----------------
	Analisis Lora
	Si se recibe un mensaje por Radio ( oLoraMensaje.lRxMensaje = 1 ), reseteamos el flag oLoraMensaje.lRxMensaje
	Confeccionamos el mensaje hacia Serverpic y lo enviamos a Serverpic
		oLoraMensaje.Remitente = El usuario de Serverpic que solicito una accion de un remoto de Lora
		oLoraMensaje.Mensaje = <Nombre remoto Lora>-:-<Acción realizada>
	------------------*/
	if (oLoraMensaje.lRxMensaje)								//Comprobamos si se ha recibido informacion por radio y si es asi le damos prioridad a la radio
	{
		oLoraMensaje.lRxMensaje = 0;							//Resetasmo el flag de informacion recibida por radio			
		oMensaje.Mensaje = oLoraMensaje.Mensaje;				//Confeccionamos el mensaje a enviar hacia el servidor	
		oMensaje.Destinatario = oLoraMensaje.Remitente;
		EnviaMensaje(oMensaje);									//Y lo enviamos
	}
	/*----------------
	Analisis Serverpic
	*/
 	oMensaje = Mensaje ();								 					 
 	if ( oMensaje.lRxMensaje)										
 	{
		if ((oMensaje.Mensaje).indexOf("#R-:-") == 0)				//Si el inicio del mensaje es #R es mensaje para enviar a los remotos
		{
			oMensaje.Mensaje = String(oMensaje.Mensaje).substring(  3 + String(oMensaje.Mensaje).indexOf("-:-"),  String(oMensaje.Mensaje).length() ); //Excluimos #R del mensaje
			TelegramaToLora(oMensaje);
		}else{
			//Procesamos el mensaje en Local
		}	
 	}
 	wdt_reset(); 													
}
```

Si se recibe mensaje de Serverpic, le pasa a la función **TelegramaToLora** el contenido del mensaje dentro de una estructura **Telegrama**. La función se encarga de convertir el telegrama en un texto y transmitirlo por la radio.

El Slave recibe ese texto oy lo trata segun el siguiente esquema

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
La radio esta en recepción permanente, si recibe un texto, la función **onRecieve** de **LoraServerpic.h** se encarga de obtener el mensaje y convertirlo a la variable **oLoraMensaje** con formato **Telegrama** con el flag **lRxMensaje** a 1, el programa principal, da prioridad a esta información sobre la que se pueda recibir en el mismo instantes desde **Serverpic** y analizaliza el Telegrama **oLoraMensaje**  de forma que el bucle de analisis de comandos recibidos por radio es similar al bucle utilizado para Serverpic.

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