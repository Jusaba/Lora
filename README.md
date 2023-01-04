# Lora
Se inicia el proyecto con dispositivo Lora junto a ESP32. El código se ha ejecutado satisfactoriamente en los módulos **LoRa 32 V2**
Se ha utilizado la libreria de **Sandeep Mistry** del sitio https://github.com/sandeepmistry/arduino-LoRa
Para manejar Lora con Serverpic se han editado una serie de funciones recogidas en el directorio LibLora de este repositorio. Estas librerias deben instalarse en el path habitual de librerias que suele ser C:\Users\<Usuario>\Documentos\Arduino\libraries. Se incluyen en este repositorio para tenerlas actualilzadas y disponibles.

## Funcionamiento
Se han incluido dos modulos distiontos, **Master** y **Slave** , el prinicipio de funcionamienrto es el siguiente:
El **Master**, en el arranque se conecta a Serverpic e inicilamente,  básicamente lo que hace es transmitir por radio las ordenes que recibe por Serverpic. El **Slave** esta como receptor, recibe por radio las ordenes recibidas del **Master**.
Para 
