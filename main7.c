#include "fsm.h"
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include <stdio.h>
#include <unistd.h>


enum semp_state {
  IDLE,
  CODE,
};

static int start = 0;
static int i = 0;
static int pulsador =0;
static int pulsado_previo=0;
static fsm_t* fsm = NULL;
static struct timeval espera_deadline;
static int code_ok=0;
static char clave[3] = {6,3,5};
static char codigo[3] = {0,0,0};

int
timeval_less (const struct timeval* a, const struct timeval* b)
{
  return (a->tv_sec == b->tv_sec)? (a->tv_usec < b->tv_usec) :
    (a->tv_sec < b->tv_sec);
}
// timeval add hace a+b
void
timeval_add (struct timeval* res,
             const struct timeval* a, const struct timeval* b)
{
  res->tv_sec = a->tv_sec + b->tv_sec;
  res->tv_usec = a->tv_usec + b->tv_usec;
  if (res->tv_usec >= 1000000) {
    res->tv_sec += res->tv_usec / 1000000;
    res->tv_usec = res->tv_usec % 1000000;
  }
}

void
timeval_sub (struct timeval* res,
             const struct timeval* a, const struct timeval* b)
{
  res->tv_sec = a->tv_sec - b->tv_sec;
  res->tv_usec = a->tv_usec - b->tv_usec;
  if (res->tv_usec < 0) {
    res->tv_sec --;
    res->tv_usec += 1000000;
  }
}

void
teclado (struct timeval* deadline)
{
	char buf[256];
	struct timeval now, timeout;
	gettimeofday (&now, NULL);
	timeval_sub (&timeout, deadline, &now);
	while (1) {
		fd_set rdset;
		FD_ZERO (&rdset);
		FD_SET (0, &rdset);
		int n = select (1, &rdset, NULL, NULL, &timeout);
		if (n == 0) { // timeout
			return;
		}
		else if (n > 0) { // algo que leer
			read (0, buf, 256);
			if (strncmp ("s", buf, 1) == 0) {
				start = 1;
				printf ("start = 1\n");
			}

			else if (strncmp ("p", buf, 1) == 0) {
				pulsador = 1;
				printf ("pulsador = 1\n");
			}
			else if (strncmp ("e", buf, 1) == 0) {
				printf (" e = %d, start = %d, pulsador = %d\n",
					fsm->current_state, start, pulsador);
			}
		}
	}
}


static int start_activado (fsm_t* this) { return start; }

static int codigo_correcto (fsm_t* this) { 
	struct timeval now;
	gettimeofday (&now, NULL);
	return ((i>=2) && (timeval_less (&espera_deadline, &now)) && ((clave[0]==codigo[0]) && (clave[1]==codigo[1]) && (clave[2]==codigo[2])));
}

static int codigo_incorrecto (fsm_t* this) { 
	struct timeval now;
	gettimeofday (&now, NULL);
	return ((i>=2) && (timeval_less (&espera_deadline, &now)) && ((clave[0]!=codigo[0]) || (clave[1]!=codigo[1]) || (clave[2]!=codigo[2]))); 
}

static int pulsador_activo (fsm_t* this) { 
	struct timeval now;
	gettimeofday (&now, NULL);
	return ((i<3) && (pulsador==1)); 
}

static void incremento_valor (fsm_t* this) { 
	static struct timeval t_espera = { 3, 0 };
	gettimeofday (&espera_deadline, NULL);
	timeval_add (&espera_deadline, &espera_deadline, &t_espera);
	codigo[i]=codigo[i]+1 % 11;
	printf("codigo[0] = %d, codigo[1] = %d, codigo[2] = %d, i= %d",codigo[0], codigo[1], codigo[2], i);
	pulsado_previo=1;
	printf ("Numero incrementado\n"); 
	pulsador = 0; 
}

static int deadline_alcanzado (fsm_t* this) { 
	struct timeval now;
	gettimeofday (&now, NULL);
	return ((i<2) && (timeval_less (&espera_deadline, &now)) && (pulsado_previo==1));
}

static void cambio_digito (fsm_t* this) { 
	static struct timeval t_espera = { 3, 0 };
	gettimeofday (&espera_deadline, NULL);
	timeval_add (&espera_deadline, &espera_deadline, &t_espera);
	i=i+1 % 3;
	printf ("Digito cambiado\n"); 
	pulsado_previo = 0; 
}

static void activo_code_ok (fsm_t* this) { 
	code_ok=1;
	printf ("Codigo acertado\n"); 
	start = 0; 
	codigo[0]=0;
	codigo[1]=0;
	codigo[2]=0;
	i=0;
}

static void desactivo_code_ok (fsm_t* this) { 
	code_ok=0;
	printf ("Codigo fallado\n"); 
	start = 0; 
	codigo[0]=0;
	codigo[1]=0;
	codigo[2]=0;
	i=0;
}

int main ()
{
	static fsm_trans_t tt[] = {
		{ IDLE, start_activado, CODE, NULL},
		{ CODE, pulsador_activo, CODE, incremento_valor },
		{ CODE, deadline_alcanzado, CODE, cambio_digito },
		{ CODE, codigo_correcto, IDLE, activo_code_ok },
		{ CODE, codigo_incorrecto, IDLE, desactivo_code_ok },
		{ -1, NULL, -1, NULL }
	};
	static struct timeval period = { 1, 0 };
	struct timeval next;
	fsm = fsm_new (tt);

	gettimeofday (&next, NULL);
	while (1) {
		fsm_fire (fsm);
		timeval_add (&next, &next, &period);
		teclado (&next);
	}
}

