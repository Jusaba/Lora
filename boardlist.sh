/**
******************************************************
* @file boardlist.sh
* @brief LoraTest
* @author Julian Salas Bartolome
* @version 1.0
* @date 12/25/2022
*
*
*******************************************************/

Genera_Parametros () {
        rm parametros.env
        touch parametros.env
        echo _ino="LoraTest" | tee parametros.env
        case $1 in
        "ESP01")
                echo _fqbn=esp8266:esp8266:generic | tee -a parametros.env
                echo _xtal=80 | tee -a parametros.env
                echo _CrystalFreq=26 | tee -a parametros.env
                echo _lvl=None____ | tee -a parametros.env
                echo _dbg=Disabled | tee -a parametros.env
                echo _wipe=none | tee -a parametros.env
                echo _sdk=nonosdk221 | tee -a parametros.env
                echo _exception=disabled | tee -a parametros.env
                echo _FlashFreq=40 | tee -a parametros.env
                echo _FlashMode=dout | tee -a parametros.env
                echo _eesz=1M | tee -a parametros.env
                echo _ip=lm2f | tee -a parametros.env
                echo _ResetMethod=ck | tee -a parametros.env
                echo _ssl=all | tee -a parametros.env
                echo _baud=115200 | tee -a parametros.env
                echo _vt=flash | tee -a parametros.env
                ;;
        "ESP01_4M")
                echo _fqbn=esp8266:esp8266:generic | tee -a parametros.env
                echo _xtal=80 | tee -a parametros.env
                echo _CrystalFreq=26 | tee -a parametros.env
                echo _lvl=None____ | tee -a parametros.env
                echo _dbg=Disabled | tee -a parametros.env
                echo _wipe=none | tee -a parametros.env
                echo _sdk=nonosdk221 | tee -a parametros.env
                echo _exception=disabled | tee -a parametros.env
                echo _FlashFreq=40 | tee -a parametros.env
                echo _FlashMode=dout | tee -a parametros.env
                echo _eesz=4M | tee -a parametros.env
                echo _ip=lm2f | tee -a parametros.env
                echo _ResetMethod=ck | tee -a parametros.env
                echo _ssl=all | tee -a parametros.env
                echo _baud=115200 | tee -a parametros.env
                echo _vt=flash | tee -a parametros.env
                ;;                
        "ESP11")
                echo _fqbn=esp8266:esp8266:generic | tee -a parametros.env
                echo _xtal=80 | tee -a parametros.env
                echo _CrystalFreq=26 | tee -a parametros.env
                echo _lvl=None____ | tee -a parametros.env
                echo _dbg=Disabled | tee -a parametros.env
                echo _wipe=none | tee -a parametros.env
                echo _sdk=nonosdk221 | tee -a parametros.env
                echo _exception=disabled | tee -a parametros.env
                echo _FlashFreq=40 | tee -a parametros.env
                echo _FlashMode=dout | tee -a parametros.env
                echo _eesz=1M | tee -a parametros.env
                echo _ip=lm2f | tee -a parametros.env
                echo _ResetMethod=ck | tee -a parametros.env
                echo _ssl=all | tee -a parametros.env
                echo _baud=115200 | tee -a parametros.env
                echo _vt=flash | tee -a parametros.env
                ;;                
        "ESP12")
                echo _fqbn=esp8266:esp8266:generic | tee -a parametros.env
                echo _xtal=80 | tee -a parametros.env
                echo _CrystalFreq=26 | tee -a parametros.env
                echo _lvl=None____ | tee -a parametros.env
                echo _dbg=Disabled | tee -a parametros.env
                echo _wipe=none | tee -a parametros.env
                echo _sdk=nonosdk221 | tee -a parametros.env
                echo _exception=disabled | tee -a parametros.env
                echo _FlashFreq=40 | tee -a parametros.env
                echo _FlashMode=dout | tee -a parametros.env
                echo _eesz=1M | tee -a parametros.env
                echo _ip=lm2f | tee -a parametros.env
                echo _ResetMethod=ck | tee -a parametros.env
                echo _ssl=all | tee -a parametros.env
                echo _baud=115200 | tee -a parametros.env
                echo _vt=flash | tee -a parametros.env
                ;;
        "ESP12_2")
                echo _fqbn=esp8266:esp8266:generic | tee -a parametros.env
                echo _xtal=80 | tee -a parametros.env
                echo _CrystalFreq=26 | tee -a parametros.env
                echo _lvl=None____ | tee -a parametros.env
                echo _dbg=Disabled | tee -a parametros.env
                echo _wipe=none | tee -a parametros.env
                echo _sdk=nonosdk221 | tee -a parametros.env
                echo _exception=disabled | tee -a parametros.env
                echo _FlashFreq=40 | tee -a parametros.env
                echo _FlashMode=dout | tee -a parametros.env
                echo _eesz=2M | tee -a parametros.env
                echo _ip=lm2f | tee -a parametros.env
                echo _ResetMethod=ck | tee -a parametros.env
                echo _ssl=all | tee -a parametros.env
                echo _baud=115200 | tee -a parametros.env
                echo _vt=flash | tee -a parametros.env
                ;;                
        esac
}


Exec_Docker () {
        docker run --rm -v /opt/docker/teamCity/teamcity_agent/serverpic/$1/librerias:/root/Arduino/libraries/serverpic -v /opt/docker/teamCity/teamcity_agent/serverpic/$1:/home/bin/esp8266 --env-file parametros.env -i jusaba/esp8266-cli:latest
}

for cLinea in $(cat hardware)
        do
                echo "------------------------"
                echo $cLinea
                echo "-------------------------"

                if [ !  -z $cLinea ];then
                        nLinea=$(grep "//Hardware Utilizado" -n ServerPic.h | cut -d ":" -f 1 )     #Buscamos la linea en ServerPic.h donde poner el hardware obtenido del fiochero harwadard
                        #------------------------------------------------------------------------------------------------
                        # Ponemos en ServerPic.h el hardware con el que compilar
                        #------------------------------------------------------------------------------------------------
                        nLinea=$((nLinea+2))                                                        #Nos ituamos dos lineas por debajo del titulo zona hardware utilizado    
                        sed -i "${nLinea}d" ServerPic.h                                             #Borramos la linea donde vamos a grabar el nombre del hardware    
                        sed -i "${nLinea} s|^|  #define  ${cLinea}\n|" ServerPic.h                  #Agregamos la linea con el hardware adecuado    
        
                        nLinea=$(grep "#ifdef ${cLinea}" -n ServerPic.h | cut -d ":" -f 1 )         #Buscamos la linea en ServerPic.h donde esta el modelo de placa leido en hardware

                        #------------------------------------------------------------------------------------------------
                        # Para ese hardware, extraemos de ServerPic.h del modelo de ESP que corresponde
                        #------------------------------------------------------------------------------------------------
                        nLineaTmp=$nLinea
                        while [  ! "$(awk  "NR==${nLineaTmp}"  ServerPic.h| grep '#define Modelo' )" ]; do
                                nLineaTmp=$((nLineaTmp+1))
                        done
                        cModelo=$(sed -n $nLineaTmp'p' ServerPic.h| cut -d " " -f 3 | sed 's/"//g')
                        cModelo="${cModelo#"${cModelo%%[![:space:]]*}"}"
                        cModelo="${cModelo%"${cModelo##*[![:space:]]}"}"
                        #------------------------------------------------------------------------------------------------
                        # Para ese hardware, exraemos de ServerPic.h el Ino que corresponde
                        #------------------------------------------------------------------------------------------------
                        nLineaTmp=$nLinea
                        while [  ! "$(awk  "NR==${nLineaTmp}"  ServerPic.h| grep '#define Ino' )" ]; do
                                nLineaTmp=$((nLineaTmp+1))
                        done
                        cIno=$(sed -n $nLineaTmp'p' ServerPic.h | cut -d " " -f 3 | sed 's/"//g')
                        cIno="${cIno#"${cIno%%[![:space:]]*}"}"
                        cIno="${cIno%"${cIno##*[![:space:]]}"}"
                        #------------------------------------------------------------------------------------------------
                        #Generamos el fichero parametros.env con los datos del modelo ESP y el nombre del ino
                        #------------------------------------------------------------------------------------------------
                        Genera_Parametros $cModelo
                        
                        #------------------------------------------------------------------------------------------------
                        #Compilamos pasandole al docker el diretorio de trabajo de TeamCity
                        #------------------------------------------------------------------------------------------------
                        Exec_Docker $1


                        cInoVersion=$cIno"_$2.bin"                                                                    #Formamos el nombre que tendra el nuevo ino con la version de compilacion realizada
                        cInoTmp="LoraTest.bin"                                                                   #Formamos el nombre del fichero compilado  
        
                        echo "-------------------------------------------------------"
                        echo $cIno
                        echo $cInoVersion
                        echo $cInoTmp
                        echo $cModelo
                        echo "-------------------------------------------------------"


                        mv $cInoTmp $cInoVersion                                                                      #Renombramos el fichero resultado de la compilacion con el nombre del ino y su version
                        curl  http://picservertest.jusaba.es:2001/firmware -F "file=@$cInoVersion"                    #Subimos el nuevo fichero con version a Serverpic 

                fi
        done


