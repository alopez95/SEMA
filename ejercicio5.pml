ltl spec1
{
    [](presencia -> <>(state==ON))
}

ltl spec2
{
    []((boton && (state==OFF)) -> <>(state==ON))
}

ltl spec3
{
    []((boton && (state==ON)) -> <>(state==OFF))
}

ltl spec4
{
    []((state==ON) && ([](!presencia)) -> <>(state==OFF))
}

mtype = {OFF, ON}; 
byte state;
bit presencia;
bit boton;

active proctype fsm () {
    state=OFF;  
    do
    ::state==OFF -> atomic{
        if
        ::presencia -> state=ON; printf("Encendido presencia \n"); presencia=0;           
        ::boton -> state=ON; printf("Encendido boton \n"); boton=0; 
        fi
    }
    ::state==ON -> atomic{
        if
        ::boton -> state=OFF; printf("Apagado \n"); boton=0; 
        ::((presencia) && (!boton)) -> state=ON; printf("Sigo encendido \n"); presencia=0;
        ::true -> state=OFF; printf("Apagado por tiempo \n");
        fi
    }
    od
}

active proctype entorno () {
    do
    ::if
        ::presencia=1;  printf("Presencia activa \n");
        ::boton=1; printf("Boton activo \n");
        ::skip -> skip;
        fi;
        printf("presencia = %d, boton = %d, state = %e\n", presencia, boton, state);
    od
}
