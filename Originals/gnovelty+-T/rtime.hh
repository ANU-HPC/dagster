/**********************************************************/
/* rtime.h, version 1.0                                   */
/*                                                        */
/* Duc Nghia Pham (d.n.pham@griffith.edu.au)              */ 
/*   IIIS, Griffith University, Australia                 */
/* February 2005                                          */
/**********************************************************/

#include <ctime>

#ifdef WIN32
	#include <sys/timeb.h>
#else
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/times.h>
#endif

int genRandomSeed();

void initTimeCounter();
double elapsedTime();
