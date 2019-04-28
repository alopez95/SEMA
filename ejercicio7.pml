ltl spec1 
{
    [] ((((clave[0]==codigo[0]) && (clave[1]==codigo[1]) && (clave[2]==codigo[2])) && (i>2) && (deadline)) -> <>(code_ok == 1))
}

ltl spec2
{
    [] ((((clave[0]!=codigo[0]) || (clave[1]!=codigo[1]) || (clave[2]!=codigo[2])) && (i>2) && (deadline)) -> <>(code_ok == 0))
}

mtype = {IDLE,CODE}; 
mtype state;
byte clave[3];
byte codigo[3];
bit code_ok;
byte i;
bit pulsador;
bit deadline;
bit start;
bit pulsador_previo;


active proctype fsm() {

    clave[0] = 6;
    clave[1] = 4;
    clave[2] = 3;
    codigo[0] = 0;
    codigo[1] = 0;
    codigo[2] = 0;
    i=0;
    pulsador=0;
    pulsador_previo=0;
    deadline=0;
    code_ok=0;
    start=0;

    
    state=IDLE;
    do

    ::state==IDLE -> atomic{
        if
        ::start  -> state=CODE; printf("Encendido \n"); start=0; //creo que no hace falta ponerlo a 0, pero por si acaso
        fi
    }
    ::state==CODE -> atomic{
        if
        ::((i<3) && (pulsador))-> codigo[i]=codigo[i]+1 % 11; printf("Sumo numero"); pulsador=0; pulsador_previo=1;
        ::((i<2) && (pulsador_previo) && (deadline)) -> i=i+1 % 3; printf("Cambio numero"); deadline=0; pulsador_previo=0; 
        ::((i>=2) && (deadline) && ((clave[0]==codigo[0]) && (clave[1]==codigo[1]) && (clave[2]==codigo[2]))) -> code_ok=1; state=IDLE; i=0; start=0; deadline=0; codigo[0]=0; codigo[1]=0; codigo[2]=0; printf("Comprobación correcta");
        ::((i>=2) && (deadline) && ((clave[0]!=codigo[0]) || (clave[1]!=codigo[1]) || (clave[2]!=codigo[2]))) -> code_ok=0; state=IDLE; i=0; start=0; deadline=0; codigo[0]=0; codigo[1]=0; codigo[2]=0; printf("Comprobación incorrecta");
        fi
    }
    od
}

active proctype entorno () {
    do
    ::if
        ::start=1; printf("Enable \n");
        ::deadline=1;  printf("Tiempo acabado \n");
        ::pulsador=1; printf("Pulsador activado \n");
        ::skip -> skip;
        fi;
        printf("pulsador = %d, deadline = %d, start= %d, i= %d\n, codigo[0] = %d, codigo[1] = %d\n, codigo[2] = %d\n", pulsador, deadline, start, i, codigo[0], codigo[1], codigo[2]);
    od
}