#include <timer_session/connection.h>
#include <base/printf.h>

/* XXX: should be the CPU session time, but the timer works for now */
static void Heure (unsigned long *tsec, unsigned long *tusec)
{
	static Timer::Connection timer;

	unsigned long ms = timer.elapsed_ms();

	*tsec = ms / 1000;
	*tusec = ms * 1000;
}


extern "C" {

#include "chrono.h"
#include <num.h>
#include <util.h>

void chrono_Init (chrono_Chrono *C) {
	Heure(&C->second, &C->microsec); }


chrono_Chrono * chrono_Create (void)
{
	chrono_Chrono *C  = new (Genode::env()->heap()) chrono_Chrono;
	Heure(&C->second, &C->microsec);
	return C;
}


void chrono_Delete (chrono_Chrono *C) { destroy(Genode::env()->heap(), C); }


double chrono_Val (chrono_Chrono *C, chrono_TimeFormat Unit)
{
   double temps;                     /* Time elapsed, in seconds */
   chrono_Chrono now;
   Heure (&now.second, &now.microsec);
   temps = (((double) now.microsec - (double) C->microsec) / 1.E+6 +
             (double) now.second) - (double) C->second;

   switch (Unit) {
   case chrono_sec:
      return temps;
   case chrono_min:
      return temps * 1.666666667E-2;
   case chrono_hours:
      return temps * 2.777777778E-4;
   case chrono_days:
      return temps * 1.157407407E-5;
   case chrono_hms:
      util_Error ("chrono_Val : hms is a wrong arg for chrono_TimeUnit");
   }
   return 0.0;
}

void chrono_Write (chrono_Chrono * C, chrono_TimeFormat Form)
{
   long centieme;
   long minute;
   long heure;
   long seconde;
   double temps;
   if (Form != chrono_hms)
      temps = chrono_Val (C, Form);
   else
      temps = 0.0;
   switch (Form) {
   case chrono_sec:
      num_WriteD (temps, 10, 2, 1);
      printf (" seconds");
      break;
   case chrono_min:
      num_WriteD (temps, 10, 2, 1);
      printf (" minutes");
      break;
   case chrono_hours:
      num_WriteD (temps, 10, 2, 1);
      printf (" hours");
      break;
   case chrono_days:
      num_WriteD (temps, 10, 2, 1);
      printf (" days");
      break;
   case chrono_hms:
      temps = chrono_Val (C, chrono_sec);
      heure = (long) (temps * 2.777777778E-4);
      if (heure > 0)
         temps -= (double) (heure) * 3600.0;
      minute = (long) (temps * 1.666666667E-2);
      if (minute > 0)
         temps -= (double) (minute) * 60.0;
      seconde = (long) (temps);
      centieme = (long) (100.0 * (temps - (double) (seconde)));
      printf ("%02ld:", heure);
      printf ("%02ld:", minute);
      printf ("%02ld.", seconde);
      printf ("%02ld", centieme);
      break;
   }
}

}
