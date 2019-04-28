
ltl spec1 
{
    []((code_ok && (state==OFF)) -> <>(state==ON))
}

ltl spec2
{
    [](((presencia && ([](!code_ok))) && (state==ON)) -> <>(sirena==1))
}

ltl spec3
{
    []((code_ok  && (state==ON)) -> <>(sirena==0))
}

ltl spec4
{
    []((code_ok && (state==ON)) -> <>(state==OFF))
}


mtype = {OFF,ON}; 
byte state;
bit presencia;
bit code_ok;
bit sirena;

active proctype fsm () {

    state=OFF;
    do
    ::state==OFF -> atomic{
        if
        ::code_ok  -> state=ON; printf("Alarma activada \n"); code_ok=0; presencia=0;
        fi
    }
    ::state==ON -> atomic{
        if
        ::((presencia) && (!code_ok)) -> state=ON; sirena=1; printf("Sirena sonando \n"); presencia=0;  
        ::code_ok -> state=OFF; printf("Alarma desactivada \n"); code_ok=0; sirena=0; 
        fi
    }
    od
}

active proctype entorno () {
    do
    ::if
        ::presencia=1;  printf("Presencia activa \n");
        ::code_ok=1; printf("Codigo_activo \n");
        ::skip -> skip;
        fi;
        printf("presencia = %d, codigo = %d, state = %e, sirena = %d\n", presencia, code_ok, state, sirena);
    od
}
