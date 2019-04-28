ltl spec1
{
    [](presencia_luz -> <>(luz_state==ON))
}

ltl spec2
{
    []((boton && (luz_state==OFF)) -> <>(luz_state==ON))
}

ltl spec3
{
    []((boton && (luz_state==ON)) -> <>(luz_state==OFF))
}

ltl spec4
{
    []((luz_state==ON) && ([](!presencia_luz)) -> <>(luz_state==OFF))
}

ltl spec5
{
    []((code_ok && (alarma_state==ALARMA_APAGADA)) -> <>(alarma_state==ALARMA_ENCENDIDA))
}

ltl spec6
{
    [](((presencia_sirena && ([](!code_ok))) && (alarma_state==ALARMA_ENCENDIDA)) -> <>(sirena==1))
}

ltl spec7
{
    []((code_ok  && (alarma_state==ALARMA_ENCENDIDA)) -> <>(sirena==0))
}

ltl spec8
{
    []((code_ok && (alarma_state==ALARMA_ENCENDIDA)) -> <>(alarma_state==ALARMA_APAGADA))
}


ltl spec9 
{
    [] ((((clave[0]==codigo[0]) && (clave[1]==codigo[1]) && (clave[2]==codigo[2])) && (i>2) && (deadline)) -> <>(code_ok == 1))
}

ltl spec10
{
    [] ((((clave[0]!=codigo[0]) || (clave[1]!=codigo[1]) || (clave[2]!=codigo[2])) && (i>2) && (deadline)) -> <>(code_ok == 0))
}


mtype = {OFF, ON, ALARMA_APAGADA, ALARMA_ENCENDIDA, IDLE, CODE}; 
mtype luz_state, alarma_state, code_state;
bit presencia_luz;
bit boton;
bit presencia_sirena;
bit code_ok;
bit sirena;
byte clave[3] = {6,3,5};
byte codigo[3] = {0,0,0};
byte i;
bit pulsador;
bit deadline;
bit start;
bit pulsador_previo;

active proctype luz () {

    
    luz_state=OFF; 

    do
    ::luz_state==OFF -> atomic{
        if
        ::presencia_luz -> luz_state=ON; printf("Encendido presencia \n"); presencia_luz=0;           
        ::boton -> luz_state=ON; printf("Encendido boton \n"); boton=0; 
        fi
    }
    ::luz_state==ON -> atomic{
        if
        ::boton -> luz_state=OFF; printf("Apagado \n"); boton=0; 
        ::((presencia_luz) && (!boton)) -> luz_state=ON; printf("Sigo encendido \n"); presencia_luz=0;
        ::true -> luz_state=OFF; printf("Apagado por tiempo \n");
        fi
    }
    od
}

active proctype alarma () {

    alarma_state=ALARMA_APAGADA;


    do
    ::alarma_state==ALARMA_APAGADA -> atomic{
        if
        ::code_ok  -> alarma_state=ALARMA_ENCENDIDA; printf("Alarma activada \n"); code_ok=0; presencia_sirena=0;
        fi
    }
    ::alarma_state==ALARMA_ENCENDIDA -> atomic{
        if
        ::((presencia_sirena) && (!code_ok)) -> alarma_state=ALARMA_ENCENDIDA; sirena=1; printf("Sirena sonando \n"); presencia_sirena=0;  
        ::code_ok -> alarma_state=ALARMA_APAGADA; printf("Alarma desactivada \n"); code_ok=0; sirena=0; 
        fi
    }
    od
}

active proctype code() {

    code_state=IDLE;
    do

    ::code_state==IDLE -> atomic{
        if
        ::start  -> code_state=CODE; printf("Encendido \n"); start=0; //creo que no hace falta ponerlo a 0, pero por si acaso
        fi
    }
    ::code_state==CODE -> atomic{
        if
        ::((i<3) && (pulsador))-> codigo[i]=codigo[i]+1 % 11; printf("Sumo numero"); pulsador=0; pulsador_previo=1;
        ::((i<2) && (pulsador_previo) && (deadline)) -> i=i+1 % 3; printf("Cambio numero"); deadline=0; pulsador_previo=0; 
        ::((i>=2) && (deadline) && ((clave[0]==codigo[0]) && (clave[1]==codigo[1]) && (clave[2]==codigo[2]))) -> code_ok=1; code_state=IDLE; i=0; start=0; deadline=0; codigo[0]=0; codigo[1]=0; codigo[2]=0; printf("Comprobación correcta");
        ::((i>=2) && (deadline) && ((clave[0]!=codigo[0]) || (clave[1]!=codigo[1]) || (clave[2]!=codigo[2]))) -> code_ok=0; code_state=IDLE; i=0; start=0; deadline=0; codigo[0]=0; codigo[1]=0; codigo[2]=0; printf("Comprobación incorrecta");
        fi
    }
    od
}

active proctype entorno () {
    do
    ::if
        ::presencia_luz=1;  printf("Presencia luz activa \n");
        ::boton=1; printf("Boton activo \n");
        ::presencia_sirena=1;  printf("Presencia sirena activa \n");
        ::start=1; printf("Enable \n");
        ::deadline=1;  printf("Tiempo acabado \n");
        ::pulsador=1; printf("Pulsador activado \n");
        ::skip -> skip;
        fi;
        printf("presencia_luz = %d, presencia_sirena = %d, boton = %d, luz_state = %d, alarma_state = %d, code_state = %d, pulsador = %d, deadline = %d, start= %d, i= %d\n, codigo[0] = %d, codigo[1] = %d\n, codigo[2] = %d\n", presencia_luz, presencia_sirena, boton, luz_state, alarma_state, code_state,  pulsador, deadline, start, i, codigo[0], codigo[1], codigo[2]);
    od
}
