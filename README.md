# Lora
Se inicia el proyecto con dispositivo Lora junto a ESP32. El código se ha ejecutado satisfactoriamente en los módulos **LoRa 32 V2**
Se ha utilizado la libreria de **Sandeep Mistry** del sitio https://github.com/sandeepmistry/arduino-LoRa
Para manejar Lora con Serverpic se han editado una serie de funciones recogidas en el directorio LibLora de este repositorio. Estas librerias deben instalarse en el path habitual de librerias que suele ser C:\Users\<Usuario>\Documentos\Arduino\libraries. Se incluyen en este repositorio para tenerlas actualilzadas y disponibles.

## Funcionamiento
Se han incluido dos modulos distiontos, **Master** y **Slave** , el prinicipio de funcionamienrto es el siguiente:
El **Master**, en el arranque se conecta a Serverpic  básicamente lo que hace es transmitir por radio las ordenes que recibe por Serverpic. El **Slave** esta como receptor, recibe por radio las ordenes recibidas del **Master**.
El **Slave** se ha intentado hacer semejante a cualquier dispositivo Serverpic, recibe un mensaje de Serverpic, crea un Telegrama y lo analiza en el loop del programa  principal para ejecutar la orden pertinente por lo tanto, las rutinas de la radio, entregarán al programa principal un Telegrama exactamente identico a los que se tratan en Serverpic.
Inicialmente se ha contemplado un solo receptor pero en futuras versiones se adaptara para disponer de varios receptores para distintos usos.

En el Master, la estructura loop del programa principal queda de la siguiente forma

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
  if ( oMensaje.lRxMensaje)										
  {
    
    TelegramaToLora(oMensaje);

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