# Lora
Se inicia el proyecto con dispositivo Lora junto a ESP32. El código se ha ejecutado satisfactoriamente en los módulos **LoRa 32 V2**
Se ha utilizado la libreria de **Sandeep Mistry** del sitio https://github.com/sandeepmistry/arduino-LoRa
Para manejar Lora con Serverpic se han editado una serie de funciones recogidas en el directorio LibLora de este repositorio. Estas librerias deben instalarse en el path habitual de librerias que suele ser C:\Users\<Usuario>\Documentos\Arduino\libraries. Se incluyen en este repositorio para tenerlas actualilzadas y disponibles.

## Funcionamiento
Se han incluido dos modulos distiontos, **Master** y **Slave** , el prinicipio de funcionamienrto es el siguiente:
El **Master**, en el arranque se conecta a Serverpic y básicamente lo que hace es esperar mensajes de Serverpic y diferenciar si son para el **Master** o para el **Slave**.
Un mensaje para el **Master** tendrá el formato habitual de Serverpic

	mensaje-:-<Master>-:-<orden>

Un menmsaje pare un **Slave** tendrá el siguiente formato en Serverpic

	mensaje-:-<Master>-:-#R-:-<Salve>-:-<orden>

				o por Broadcast

	mensaje-:--<Master>-:-#R-:-broadcast-:-<orden>  	

El **Master**, si es una orden para el actuará en consecuencia, si es una orden para un **Slave**, elaborará un mensaje similar a los de Serverpic y lo enviará por radio. El mensaje tendra el siguiente formato

	<Remitente>-:-<Salve>-:-<orden>

					o

	<Remitente>-:-broadcast-:-<orden>				


El **Slave** se ha intentado hacer semejante a cualquier dispositivo Serverpic pero teniendo en cuanta que los telegramas ahora los recibirá por radio desde el **Masater**. Las rutinas de recepción de la radio estarán en espera de recepción y cuando se reciba un telegrama pondrá a **1** el flag  **oLoraMensaje.lRxMensaje** y cederá el análisis al loop del programa principal.
En el programa principal, se analiza si el telegrama va dirigido a este **Slave** o si es de broadcast, si es así ejecutará la orden recibida y si no la ignorará

En la otra dirección, el **Slave**, si tiene que responder a la orden recibida, elaborara un mensaje de la siguiente forma, supongamos que el **Master** recibe el mensaje siguiente

	mensaje-:-<Master>-:-#R-:-<Salve>-:-<orden>

El **Master** manda a **Slave** el siguiente mensaje

	<Remitente>-:-<Salve>-:-<orden>

El **Slave** manda por radio el siguiente texto

	<Remitente>-:-<Salve>-:-<respuesta>

El **Master** recibe ese texto y elabora el siguiente mensaje

	mensaje-:-<Remitente>-:-<Salve>-:-<respuesta>

Con esto, el usuario de Serverpic que mandó una orden a un **Salve** recibirá la respuesta desde el **Slave** a esa orden.	



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

Si se recibe mensaje de Serverpic, le pasa a la función **TelegramaToLora** el contenido del mensaje dentro de una estructura **Telegrama**. La función se enbcarga de convertir el telegrama en un texto y transmitirlo por la radio.

El Slave recibe ese texto o lo trata segun el siguiente esquema

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
    	
  oMensaje = Mensaje ();	
  if (oLoraMensaje.lRxMensaje)	
  {
			oMensaje = oLoraMensaje;
			oLoraMensaje.lRxMensaje = 0;
  }								 					 
  if ( oMensaje.lRxMensaje)										
  {
     //*******************************************************************************
     //En este punto empieza el bloque de programa particular del dispositivo según la 
     //utilización
     //Se analiza el mensaje recibido y se actúa en consecuencia
     //*******************************************************************************
  }
  wdt_reset(); 													
}
```
La radio esta en recepción permanente, si recibe un texto, la función **onRecieve** de **LoraServerpic.h** se encarga de obtener el mensaje y convertirlo a la variable **oLoraMensaje** con formato **Telegrama** con el flag **lRxMensaje** a 1, el programa principal, da prioridad a esta información sobre la que se pueda recibir en el mismo instantes desde **Serverpic** y actualiza el Telegrama **oMensaje** con la informacion de **oLoraMensaje** de forma que el bucle de analisis de comandos recibidos tratara esa información como si lo recibiera de Serverpic