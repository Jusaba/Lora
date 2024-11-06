/**
******************************************************
* @file TUF2000.h
* @brief Libreria para gestrion de medidor ultrasonico TUF2000M
* @author Julian Salas Bartolome
* @version 1.0
* @date 21/10/2024
*
* Necesita las librerias:
*
*    ModbusMaster   en  https://github.com/4-20ma/ModbusMaster
*    SoftwareSerial en h ttps://github.com/junhuanchen/Esp32-SoftwareSerial/tree/master
*   https://www.libe.net/en/flowmeter
*******************************************************/

#include <SoftwareSerial.h>
#include <ModbusMaster.h>



#ifndef TUF2000M_H
	#define TUF2000M_H


#define FLOW_DATA_SIZE 2
#define LONG_DATA_SIZE 2

#define FLOAT_DATA_SIZE  2

#define Registro_Flow                          1
#define Registro_PositiveAcumulatorDecimal    11
#define Registro_Menu	                        60
#define Registro_Tecla	                      59
#define Registro_DateTime                     53
#define RegistroFlowForTodayDecimal          139
#define RegistroFlowForMonthDecimal          143
#define RegistroFlowForYearDecimal           147

#define Menu_0                            0x0000                    //Menu 0
#define MenuOuterDiameterPipe             0x0011                    //Menu 11
#define MenuThicknessPipe                 0x0012                    //Menu 12
#define MenuMaterialPipe                  0x0014                    //Menu 14
#define Menu_Idioma		                    0x0039                    //Menu Idioma
#define Menu_DateTime                     0x0060                    //Menu Fecha/Hora
#define Menu_Unidades                     0x0031

#define Metros3             0
#define Litros              1
#define Ingles		          3	   		            //Ingles
#define Cobre               4

#define Hora                0
#define Dia                 1
#define Segundo             2
#define Minuto              3




#define Key_Enter 		0x003D                    //Enter


//Configuracion
int Idioma = Ingles;
int Unidades = Litros;
int UnidaddeTiempo = Hora;

//Datos tuberia
int OuterDiameter = 355;
int thickness = 3175;
int InnerDiameter = 0;
int Material = Cobre;                         


#define DebugTuf

ModbusMaster TUF;


void EnterKey (int nKey);
void WindowMenu (int nMenu);

void ConfiguraIdioma (void);
void ConfiguraHoraFecha (  int nSg, int nMinutos, int nHora, int nDia, int nMes, int nAno );
void ConfiguraUnidades (void);
void ConfiguraPipe (void);
void ConfiguraOuterDiameterPipe (void);
void ConfiguraThicknessPipe (void);
void ConfiguraMaterialPipe (void);


int IntToBcd (int nDato );
void WriteNumber ( int nNumero );
float LeeRegistrosFloat ( int nRegistro );
long LeeRegistrosLong ( int nRegistro );

float readFlow(void);
float ReadPositiveAcumulator (void);
float FlowForTodayDecimal (void);
float FlowForMonthDecimal (void);
float FlowForYearDecimal (void);

/**
  * @brief Lee el dato long de un par de registros
  * 
  * @param nRegistro.- Primer registro a leer
  * 
  * @return Devuelve el long almacenado en el par de registros
  */
  long LeeRegistrosLong ( int nRegistro )
  {
      uint16_t buf[FLOAT_DATA_SIZE];
      uint8_t j, result;
      uint16_t temp;
      long DatoSalida = -1;
      result = TUF.readHoldingRegisters(nRegistro, LONG_DATA_SIZE);
      if (result == TUF.ku8MBSuccess)
      {
        for (j = 0; j < FLOAT_DATA_SIZE; j++)
        {
          buf[j] = TUF.getResponseBuffer(j);
        }    
        temp = buf[1];
        buf[1]=buf[0];
        buf[0]=temp;
        memcpy(&DatoSalida, &buf, sizeof(long));       
        #ifdef DebugTuf
          Serial.print ("LeeRegistrosLong -> Dato Leido:  ");
          Serial.println (DatoSalida);
        #endif  
      }else{
        #ifdef DebugTuf
          Serial.println ("LeeRegistrosLong -> Error de lectura  ");
        #endif  
      }
      return (DatoSalida);
    }


  /**
  * @brief Lee el dato float de un par de registyros
  * 
  * @param nRegistro.- Primer registro a leer
  * 
  * @return Devuelve el float almacenado en el par de registros
  */
  float LeeRegistrosFloat ( int nRegistro )
  {
      uint16_t buf[FLOAT_DATA_SIZE];
      uint8_t j, result;
      uint16_t temp;
      float DatoSalida = -1;
      result = TUF.readHoldingRegisters(nRegistro, FLOW_DATA_SIZE);
      if (result == TUF.ku8MBSuccess)
      {
        for (j = 0; j < FLOAT_DATA_SIZE; j++)
        {
          buf[j] = TUF.getResponseBuffer(j);
        }    
        temp = buf[1];
        buf[1]=buf[0];
        buf[0]=temp;
        memcpy(&DatoSalida, &buf, sizeof(float));       
        #ifdef DebugTuf
          Serial.print ("LeeRegistrosFloat -> Dato Leido:  ");
          Serial.println (DatoSalida);
        #endif  
      }else{
        #ifdef DebugTuf
          Serial.println ("LeeRegistrosFloat -> Error de lectura  ");
        #endif  
      }
      return (DatoSalida);
    }


/**
* @brief Va a una ventana de Menu
*
* @param nMenu.- Ventana del menu al que se quiere acceder
*/
void WindowMenu (int nMenu)
{
   uint8_t result;
   result = TUF.writeSingleRegister (Registro_Menu-1, nMenu);            //Ventana de menu nMenu
} 
/**
* @brief Configura el idioma a Ingles
*/
void ConfiguraIdioma (void)
{
	uint8_t result;
  
  WindowMenu(Menu_Idioma);                                                   //Pantalla idioma
  delay(10);  
  if (result == TUF.ku8MBSuccess)
  {
      EnterKey(Key_Enter);                                                  //<Ent>
      delay(10);    
      EnterKey(Idioma);                                                     //Ingles
      delay(10);
      EnterKey(Key_Enter);
      delay(10);
      WindowMenu (Menu_0);
      delay(10);
   	  #ifdef DebugTuf
        Serial.println("ConfiguraIdioma -> Cambiado idioma");
      #endif  
	}else{
      #ifdef DebugTuf
		    Serial.println("ConfiguraIdioma -> No ha sido posible cambiar el idioma");
      #endif  
	}	
}
/**
******************************************************
* @brief Pone Hora, Minutos, Segundos, dia, mes y año en RTC
*
* @param nSg.- Segundos
* @param nMinutos.- Minutos
* @param nHora.- Horas
* @param nDia.- Dia
* @param nMes.- Mes
* @param nAno.- Año
*/
void ConfiguraHoraFecha (  int nSg, int nMinutos, int nHora, int nDia, int nMes, int nAno )
{
    int Temp = 0;
    int nYear = nAno % 100;

    Temp =  IntToBcd(nMinutos) << 8;
    Temp =  Temp + IntToBcd(nSg);
    TUF.setTransmitBuffer(0, Temp);
    Temp = 0;
    Temp = IntToBcd(nDia) << 8;
    Temp = Temp + IntToBcd(nHora);
    TUF.setTransmitBuffer(1, Temp);
    Temp = 0;
    Temp = IntToBcd(nYear) << 8;
    Temp = Temp + IntToBcd(nMes);
    TUF.setTransmitBuffer(2, Temp);
    TUF.writeMultipleRegisters (Registro_DateTime-1, 3);

    TUF.writeSingleRegister (Registro_Menu-1, Menu_DateTime);            //Pantalla Fecha/Hora

}
/**
******************************************************
* @brief Configura las unidades de medida caudal con las unidades almacenadas e Unidades y UnidaddeTiempo
*
* Los seleccionados serán l/h o m3/h
*/
void ConfiguraUnidades (void)
{
  uint8_t result;
  WindowMenu(Menu_Unidades);                                                   //Pantalla idioma
  delay(10);  
  if (result == TUF.ku8MBSuccess)
  {
      EnterKey(Key_Enter);                                                  //<Ent>
      delay(10);    
      EnterKey(Unidades);
      delay(10);
      EnterKey(Key_Enter);                                                  //<Ent>
      delay(10);    
      EnterKey(UnidaddeTiempo);
      delay(10);
      EnterKey(Key_Enter);                                                  //<Ent>
      delay(10);    
     	#ifdef DebugTuf
        Serial.println("ConfiguraUnidades -> Cambiado Unidades");
      #endif  
	}else{
      #ifdef DebugTuf
		    Serial.println("ConfiguraUnidades -> No ha sido posible cambiar las unidades");
      #endif  
	}	
}  
void WriteNumber ( int nNumero, int nDecimales )
{
    int nDigito;
    int nNumeroTmp = nNumero;
    int nDecimal = 0;

    nDigito = int(nNumeroTmp/1000);
    EnterKey(nDigito);
    delay(10);

    if ( nDecimales == 3 )
    {
        EnterKey(0x3A);
        delay(10);
    }

    nNumeroTmp = nNumeroTmp - ( nDigito * 1000);
    nDigito = int(nNumeroTmp/100);
    EnterKey(nDigito);
    delay(10);

    if ( nDecimales == 2 )
    {
        EnterKey(0x3A);
        delay(10);
    }
 
    nNumeroTmp = nNumeroTmp - ( nDigito * 100);
    nDigito = int(nNumeroTmp/10);
    EnterKey(nDigito);
    delay(10);

    if ( nDecimales == 1 )
    {
        EnterKey(0x3A);
        delay(10);
    }
 
    nNumeroTmp = nNumeroTmp - ( nDigito * 10);
    EnterKey(nNumeroTmp);
    delay(10);

}

void ConfiguraPipe (void)
{
  ConfiguraOuterDiameterPipe ();
  ConfiguraThicknessPipe ();
  ConfiguraMaterialPipe ();
}

void ConfiguraOuterDiameterPipe (void)
{
    uint8_t result;
    WindowMenu(MenuOuterDiameterPipe);                                                   //Pantalla Outer Diameter Pipe
    delay(10);  
    if (result == TUF.ku8MBSuccess)
    {
      EnterKey(Key_Enter);                                                              //<Ent>
      delay(10);

      WriteNumber(OuterDiameter, 1);

      EnterKey(Key_Enter);                                                  //<Ent>
      delay(10);
      EnterKey(Key_Enter);                                                  //<Ent>
      delay(10);

    } 
}
void ConfiguraThicknessPipe (void)
{
    uint8_t result;
    WindowMenu(MenuThicknessPipe);                                                   //Pantalla Thickness Pipe
    delay(10);  
    if (result == TUF.ku8MBSuccess)
    {
      EnterKey(Key_Enter);                                                              //<Ent>
      delay(10);

      WriteNumber(thickness, 3);

      EnterKey(Key_Enter);                                                  //<Ent>
      delay(10);
      EnterKey(Key_Enter);                                                  //<Ent>
      delay(10);
      EnterKey(Key_Enter);                                                  //<Ent>
      delay(10);

    } 
}
void ConfiguraMaterialPipe (void)
{
    uint8_t result;
    WindowMenu(MenuMaterialPipe);                                                      //Pantalla Material Pipe
    delay(10);  
    if (result == TUF.ku8MBSuccess)
    {
      EnterKey(Key_Enter);                                                              //<Ent>
      delay(10);

      EnterKey(Material);                                                               //Cobre
      delay(10);

      EnterKey(Key_Enter);                                                              //<Ent>
      delay(10);


    }  
}

/**
* @brief Lee el registro 'flow RATE' (0001) 
* 
*/
float readFlow(void) {

    float flow = LeeRegistrosFloat(Registro_Flow);

    #ifdef DebugTuf
      if ( flow != -1) 
      {
        Serial.print("readFlow-> El caudal es:  ");
        Serial.println(flow, 6);
      }else{       
        Serial.println("readFlow-> Lectura erronea ");
      }  
    #endif
    return (flow);
  
  }
/**
* @brief Lee el registro 'positive acumulator' (0009) 
* 
*/  
float ReadPositiveAcumulator (void)
{
    float Dato = LeeRegistrosFloat(Registro_PositiveAcumulatorDecimal);

    #ifdef DebugTuf
      if ( Dato != -1) 
      {
        Serial.print("ReadPositiveAcumulator-> El acumulado es:  ");
        Serial.println(Dato, 6);
      }else{       
        Serial.println("ReadPositiveAcumulator-> Lectura erronea ");
      }  
    #endif
    return (Dato);
}  
/**
* @brief Lee el registro 'flow for today decimal fraction' (0139) 
*/
float FlowForTodayDecimal (void)
{
    float nDato = LeeRegistrosFloat (RegistroFlowForTodayDecimal);
    #ifdef DebugTuf
		  Serial.print("FlowForTodayDecimal -> Dato Leido: ");
      Serial.println(nDato);
    #endif  
    return (nDato);
}
/**
* @brief Lee el registro 'flow for this month decimal fraction' (0143) 
*/
float FlowForMonthDecimal (void)
{
    float nDato = LeeRegistrosFloat (RegistroFlowForMonthDecimal);
    #ifdef DebugTuf
		  Serial.print("FlowForMonthDecimal -> Dato Leido: ");
      Serial.println(nDato);
    #endif  
    return (nDato);
}
/**
* @brief Lee el registro 'flow for this year decimal fraction' (0147) 
*/
float FlowForYearDecimal (void)
{
    float nDato = LeeRegistrosFloat (RegistroFlowForYearDecimal);
    #ifdef DebugTuf
		  Serial.print("FlowForYearDecimal -> Dato Leido: ");
      Serial.println(nDato);
    #endif  
    return (nDato);
}


/**
* @brief Simula la pulsacion de una tecla en el teclado fronatal
* @param nKey.- Valor de la tecla que se quiere pulsar segun tabla Key Value del apartado 7.4 de especificaciones de TUF-2000M
*/
void EnterKey (int nKey)
{
  uint8_t  result;
  result = TUF.writeSingleRegister (Registro_Tecla-1,nKey);                 //Ponemos en registro de pulsación la tecla que se desea simular
  if (result == TUF.ku8MBSuccess)
  {
    #ifdef DebugTuf
      Serial.println ("EnterKey -> Tecla aceptada ");
    #endif  
  }else{
    #ifdef DebugTuf
      Serial.println ("EnterKey -> Tecla no aceptada ");
    #endif  
  }
}

/**
* @brief Convierte un numero decimal en el correposndiente BCD
* Dato XY ->nibble Alto X nible bajo Y
* @param Dato.- Dato a convertir
* @return Devuelve Dato convertido a BCD 
*/
int IntToBcd (int nDato )
{
    int nCociente, nResto;

    nCociente = nDato/10;
    nResto = nDato % 10;

    return ( (nCociente*16) + nResto );
}

#endif