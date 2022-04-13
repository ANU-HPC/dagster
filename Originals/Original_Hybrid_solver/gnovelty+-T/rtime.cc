/**********************************************************/
/* rtime.c, version 1.0                                   */
/*                                                        */
/* Duc Nghia Pham (d.n.pham@griffith.edu.au)              */ 
/*   IIIS, Griffith University, Australia                 */
/*                                                        */
/* February 2005                                          */
/**********************************************************/
/*    Consult legal.txt for legal information			  */
/**********************************************************/
/* Partial of the code is based on UBCSAT version 1.0     */
/* written by:                                            */
/* Dave A.D. Tompkins & Holger H. Hoos                    */
/*   Univeristy of British Columbia                       */
/* February 2004                                          */
/**********************************************************/

#include "rtime.hh"

double startTimeStamp;
double currentTimeStamp;

#ifdef WIN32
	struct _timeb tstruct;
#else
// #include <sys/time.h>
// #include <sys/times.h>
	struct timeval tv;
	struct timezone tzp;
	struct tms prog_tms;
#endif

#ifdef WIN32

	void getCurrentTime() {
		_ftime( &tstruct );
		currentTimeStamp = ((double) tstruct.time + ((double) tstruct.millitm) / 1000.0);
	}
	
	double elapsedTime() {
		getCurrentTime();
		return (currentTimeStamp - startTimeStamp);
	}

	int genRandomSeed() {
		_ftime( &tstruct );
		return ((( tstruct.time & 0x001FFFFF ) * 1000) + tstruct.millitm);
	}
	
#else

	void getCurrentTime() {
		times(&prog_tms);
		currentTimeStamp = (double) prog_tms.tms_utime;
	}
	
	double elapsedTime() {
		double answer;
		
		//times(&prog_tms);
		//answer = ( ((double) prog_tms.tms_utime-startTimeStamp) / ((double) sysconf(_SC_CLK_TCK)) );
		getCurrentTime();
		answer = ( (currentTimeStamp-startTimeStamp) / ((double) sysconf(_SC_CLK_TCK)) );
		return answer;
	}

	int genRandomSeed() {
		gettimeofday(&tv, &tzp);
		return ((( tv.tv_sec & 0x000007FF ) * 1000000) + tv.tv_usec);
	}
	
#endif

void initTimeCounter() {
	getCurrentTime();
	startTimeStamp = currentTimeStamp;
}

