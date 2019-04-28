#include "fsm.h"
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include <stdio.h>
#include <unistd.h>


enum semp_state {
  OFF,
  ON,
};

static int boton = 0;
static int presencia = 0;
static fsm_t* fsm = NULL;
static struct timeval espera_deadline;


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
			if (strncmp ("p", buf, 1) == 0) {
				presencia = 1;
				printf ("presencia = 1\n");
			}
			else if (strncmp ("b", buf, 1) == 0) {
				boton = 1;
				printf ("boton = 1\n");
			}
			else if (strncmp ("s", buf, 1) == 0) {
				printf (" s = %d, boton = %d, presencia = %d\n",
					fsm->current_state, boton, presencia);
			}
		}
	}
}


static int detecto_presencia (fsm_t* this) { return (presencia==1); }

static int detecto_boton (fsm_t* this) { return (boton==1); }

static int detecto_nueva_presencia (fsm_t* this) {
	return ((presencia==1) && (boton==0));
}

static int timeout (fsm_t* this) { 
	struct timeval now;
	gettimeofday (&now, NULL);
	return timeval_less (&espera_deadline, &now); 
}

static void encender_luz (fsm_t* this) { 
	static struct timeval t_espera = { 60, 0 };
	gettimeofday (&espera_deadline, NULL);
	timeval_add (&espera_deadline, &espera_deadline, &t_espera);
	printf ("Luz encendida\n"); 
	boton = 0; 
	presencia = 0; 
}
static void apagar_luz (fsm_t* this) { printf ("Luz apagada\n"); boton = 0; }

static void sigue_luz (fsm_t* this) { 
	static struct timeval t_espera = { 60, 0 };
	gettimeofday (&espera_deadline, NULL);
	timeval_add (&espera_deadline, &espera_deadline, &t_espera);
	printf ("La luz sigue encendida\n"); 
	presencia = 0; 
	 }

int main ()
{
	static fsm_trans_t tt[] = {
		{ OFF, detecto_presencia , ON, encender_luz },
		{ OFF, detecto_boton, ON, encender_luz },
		{ ON, detecto_nueva_presencia, ON, sigue_luz },
		{ ON, detecto_boton, OFF, apagar_luz },
		{ ON, timeout, OFF, apagar_luz },
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

