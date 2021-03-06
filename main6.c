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

static int sirena = 0;
static int presencia = 0;
static int code_ok = 0;
static fsm_t* fsm = NULL;

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
			else if (strncmp ("c", buf, 1) == 0) {
				code_ok = 1;
				printf ("codigo = 1\n");
			}
			else if (strncmp ("s", buf, 1) == 0) {
				printf (" s = %d, codigo = %d, presencia = %d\n",
					fsm->current_state, code_ok, presencia);
			}
		}
	}
}


static int detecto_presencia (fsm_t* this) { return ((presencia==1) && (!code_ok)); }

static int codigo_correcto (fsm_t* this) { return (code_ok==1); }


static void alarma_activada (fsm_t* this) { printf ("Alarma activada\n"); code_ok = 0; presencia = 0; }
static void encender_sirena (fsm_t* this) { printf ("Sirena encendida\n"); presencia = 0; sirena= 1; }
static void alarma_desactivada (fsm_t* this) { printf ("Alarma desactivada\n"); code_ok = 0; sirena= 0; }

int main ()
{
	static fsm_trans_t tt[] = {
		{ OFF, codigo_correcto , ON, alarma_activada },
		{ ON, detecto_presencia, ON, encender_sirena },
		{ ON, codigo_correcto, OFF, alarma_desactivada },
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

