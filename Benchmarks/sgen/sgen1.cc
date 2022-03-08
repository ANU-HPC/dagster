/*

A generator for satisfiability instances.

Author: Ivor Spence
Date: October 2009

Usage:

sgen1 -n num-of-variables [-sat | -unsat] [-s random-seed] [-m satisfying-model-file] [-min-variables] [-reorder]


*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Define command-line options

#define SAT 0
#define UNSAT 1
#define REORDER 2
#define HELP 3
#define MIN_VARIABLES 4
#define MAX_BOOLEAN_OPTIONS 5
int *booleanOptions;


#define MODEL 0
#define MAX_STRING_OPTIONS 1
char **stringOptions;

#define NUM_VARIABLES 0
#define RANDOM_SEED 1
#define MAX_INT_OPTIONS 2
int *intOptions;

int *model;
int numOfVariables, isSat, groups, reorder;
int groupSize;


#define MYRAND_MAX 32767

char  **booleanOptionTags, **stringOptionTags, **intOptionTags;
char commandLine[1000];

void initOptions()
{
	
	booleanOptionTags = (char **) malloc (MAX_BOOLEAN_OPTIONS*sizeof (char *));
	booleanOptions = (int *) malloc (MAX_BOOLEAN_OPTIONS*sizeof(int));

	booleanOptionTags[SAT] = "sat";
	booleanOptions[SAT] = 0;
	booleanOptionTags[UNSAT] = "unsat";
	booleanOptions[UNSAT] = 0;
	booleanOptionTags[REORDER] = "reorder";
	booleanOptions[REORDER] = 0;
	booleanOptionTags[HELP] = "help";
	booleanOptions[HELP] = 0;
	booleanOptionTags[MIN_VARIABLES] = "min-variables";
	booleanOptions[MIN_VARIABLES] = 0;

	intOptionTags = (char **) malloc (MAX_INT_OPTIONS*sizeof (char *));
	intOptions = (int *) malloc (MAX_INT_OPTIONS*sizeof(int));

	intOptionTags[NUM_VARIABLES] = "n";
	intOptions[NUM_VARIABLES] = 0;
	intOptionTags[RANDOM_SEED] = "s";
	intOptions[RANDOM_SEED] = 1;

	stringOptionTags = (char **) malloc (MAX_STRING_OPTIONS*sizeof (char *));
	stringOptions = (char **) malloc (MAX_STRING_OPTIONS*sizeof(char *));

	stringOptionTags[MODEL] = "m";
	stringOptions[MODEL] = NULL;
 
}

int checkBooleanOption (char *option)
{
	int o;

	for (o=0;o<MAX_BOOLEAN_OPTIONS;o++)
	{
		if (strcmp (booleanOptionTags[o], &option[1]) == 0)
		{
			booleanOptions[o] = 1;
			return 1;
		}
	}

	if (strlen (option) > 3 && option[1] == 'n' && option[2] == 'o')
		for (o=0;o<MAX_BOOLEAN_OPTIONS;o++)
		{
			if (strcmp (booleanOptionTags[o], &option[3]) == 0)
			{
				booleanOptions[o] = 0;
				return 1;
			}
		}
	return 0;
}

int checkStringOption (char *option, char *val)
{
	int o;

	for (o=0;o<MAX_STRING_OPTIONS;o++)
	{
		if (strcmp (stringOptionTags[o], &option[1]) == 0)
		{
			stringOptions[o] = val;
			return 1;
		}
	}

	return 0;
}

int checkIntOption (char *option, char *val)
{
	int o;

	for (o=0;o<MAX_INT_OPTIONS;o++)
	{
		if (strcmp (intOptionTags[o], &option[1]) == 0)
		{
			intOptions[o] = atoi (val);
			return 1;
		}
	}

	return 0;
}

void printUsage()
{
	fprintf (stderr, "Usage: sgen1 -n num-of-variables [-sat | -unsat] [-s random-seed] [-m satisfying-model-file] [-min-variables] [-reorder]\n");
	exit(0);
}

void printHelp()
{
	int i;

	for (i=0;i<MAX_BOOLEAN_OPTIONS;i++)
		fprintf (stdout, "-[no]%s\n", booleanOptionTags[i]);
	fprintf (stdout, "\n");

	for (i=0;i<MAX_INT_OPTIONS;i++)
		fprintf (stdout, "-%s int\n", intOptionTags[i]);
	fprintf (stdout, "\n");

	for (i=0;i<MAX_STRING_OPTIONS;i++)
		fprintf (stdout, "-%s string\n", stringOptionTags[i]);
	fprintf (stdout, "\n");

}

void getOptions (int *argc, char * argv[])
{
	char **newArgv;
	int oldp, newp, p, pos;

	strcpy (commandLine, "Command: ");
	pos=strlen(commandLine);
	for (p=0; p<*argc && pos+1+strlen(argv[p])<1000;p++)
	{
		strcat (commandLine, argv[p]);
		strcat (commandLine, " ");
		pos += 1 + strlen (argv[p]);
	}


	initOptions();

	newArgv = (char **) malloc ( (*argc)*sizeof(char *));

	oldp = 0;
	newp = 0;

	newArgv[newp++] = argv[oldp++];

	while (oldp < *argc)
	{
		if (strlen (argv[oldp]) == 0 || argv[oldp][0] != '-')
			newArgv[newp++] = argv[oldp++];
		else
		{
			if (checkBooleanOption (argv[oldp]))
				oldp++;
			else
				if (oldp+1 < *argc && checkStringOption (argv[oldp],argv[oldp+1]))
					oldp += 2;
			else
				if (oldp+1 < *argc && checkIntOption (argv[oldp],argv[oldp+1]))
					oldp += 2;
			else
			{
				fprintf (stderr, "Unrecognised option %s\n", argv[oldp]);
				oldp++;
			}
		}

	}

	for (p=0;p<newp;p++)
		argv[p] = newArgv[p];
	*argc = newp;

	free (newArgv);

	if (booleanOptions[HELP])
	{
		printHelp();
		exit(1);
	}
	

}

void checkOptions()
{
	if (booleanOptions[SAT] && booleanOptions[UNSAT] || !booleanOptions[SAT] && !booleanOptions[UNSAT])
	{
		fprintf (stderr, "You must specify either -sat or -unsat\n");
		printUsage();
	}
	
	if (intOptions[NUM_VARIABLES] <= 0)
	{
		fprintf (stderr, "You must specify num-of-variables to be at least 1\n");
		printUsage();
	}
	
	if (booleanOptions[UNSAT] && stringOptions[MODEL] != NULL)
	{
		fprintf (stderr, "You have specified -unsat and -m, but no model can be created for an unsatisfiable instance\n");
		printUsage();
	}
}

static unsigned long next = 1;

int myrand(void)
{
	next = next * 1103515245 + 12345;
	return((unsigned)(next/65536) % 32768);
}

void mysrand(unsigned seed)
{
	next = seed;
}






void heading(int n)
{
	int clauses;

	if (isSat)
				clauses = groups*12;
	else
				clauses = 2*(4*groups+6);
	   
		
	printf ("c generated by sgen1\n");
	printf ("c %s\n", commandLine);
	printf ("p cnf %d %d\n", n, clauses);
	
}

int *permute, *unpermute, *globalPermute;

void setPermute(int sign)
{
	int i,j,k,l;
	
	for (i=1;i<=numOfVariables;i++)
		permute[i] = 0;
		
	for (i=1;i<=numOfVariables;i++)
	{
		j = myrand() % (numOfVariables+1-i) + 1;
		k = 1;
		for (l=0;l<j;l++)
		{
			if (k == numOfVariables)
				k = 1;
			else
				k++;
			while (permute[k] != 0)
			{
				if (k == numOfVariables)
					k = 1;
				else
					k++;
			}
		}
		permute[k] = sign*i;
	}
	

}


void setPlainPermute ()
{
	int i;
	for (i=1;i<=numOfVariables;i++)
		permute[i] = i;
}

int beatProbability (double power)
{
	int result;
	double p;

	if (power > 0.0)
		result = 1;
	else
	{
		p = exp (power);
		if (p >= 1)
			result = 1;
		else
		{
			result = myrand() < p*MYRAND_MAX;
		}
	}
	return result;
}

int groupFor (int p1)
{
	if (!isSat && p1 == numOfVariables)
		return (p1-2) / groupSize;
	else
		return (p1-1)/groupSize;
}

int areNeighbours (int p1, int p2)
{
	int g1, g2;
	
	return groupFor (p1) == groupFor (p2);
}

int absolute (int i)
{
	return i>0?i:-i;
}

int neighbours (int p1, int pp1)
{
	int result = 0;
	int i,j, k, g1, gp1, p2, g2, p3, pp3,g3;
	

	g1 = groupFor (p1);
 
	gp1 = groupFor (pp1);
	
	for (i=1;i<=groupSize;i++)
	{
		p2 = unpermute[groupSize*gp1+i];
		g2 = groupFor (p2);
		if (pp1 != groupSize*gp1+i &&  (g2 == g1))
		{
			 result+=16;
		}
			
		for (j=1; j<=groupSize; j++)
		{
			p3 = groupSize*g1+j;
			if (p1 != p3)
			{
				pp3 = permute[p3];
				g3 = groupFor(pp3);
				for (k=1;k<=groupSize;k++)
					if (groupSize*g3+k != pp3 && groupFor(unpermute[groupSize*g3+k]) == g2)
						result += 1;
			}
		}
					
		
	}
			
	if (!isSat && g1 == (numOfVariables-1)/groupSize && p1 != numOfVariables)
		if (groupFor (unpermute[numOfVariables]) == g1)
			result+=16;
   

 

	return result; 
}

double swapGain (int p1, int p2)
{
	return (double) (neighbours (p1, permute[p1]) + neighbours (p2, permute[p2])) -
		(neighbours (p1, permute[p2]) + neighbours (p2, permute[p1]));
}

int totalScore()
{
	int result = 0;
	int i;
	
	for (i=1;i<=numOfVariables;i++)
		result += neighbours(i,permute[i]);
		
	return result;
}

void setAnnealingPermute ()
{
	int i,j, p1, p2, t;
	double temperature;
	double gain;
	
	for (i=1;i<=numOfVariables;i++)
	{
		permute[i] = i;
		unpermute[i] = i;
	}
	
	temperature = 20;
	
	while (temperature > 0.2)
	{
		//printf ("S=%d\n", totalScore());
		for (i=1;i<=2*numOfVariables;i++)
		{
			p1 = 1 + myrand() % numOfVariables;
			p2 = 1 + myrand() % numOfVariables;
			//printf ("%d %d\n", p1, p2);
			gain = swapGain (p1, p2);
			//printf ("%f ", gain);
			if (beatProbability (gain/temperature))
			{
				t = permute[p1];
				permute[p1] = permute[p2];
				permute[p2] = t;
				unpermute[permute[p1]] = p1;
				unpermute[permute[p2]] = p2;
				 
			}
	   }
		fflush (stdout);
		temperature *= 0.995;
	 }
	

}


void setAnnealingPermute1 ()
{
	int i,j, p1, p2, t;
	double temperature;
	double gain;
	
	for (i=1;i<=numOfVariables;i++)
	{
		permute[i] = i;
		unpermute[i] = i;
	}
	
	temperature = 20;
	
	while (temperature > 0.2)
	{
		for (i=1;i<=2*numOfVariables;i++)
		{
			do
			{
				p1 = 1 + myrand() % numOfVariables;
			} while (model[p1]== 1);
			
			do
			{
				p2 = 1 + myrand() % numOfVariables;
			} while (model[p2] == 1);
				
		   gain = swapGain (p1, p2);
			if (beatProbability (gain/temperature))
			{
				t = permute[p1];
				permute[p1] = permute[p2];
				permute[p2] = t;
				unpermute[permute[p1]] = p1;
				unpermute[permute[p2]] = p2;
				
			}
		}
		fflush (stdout);
		temperature *= 0.995;
	 }
   

}



void lastUnsatGroup(int sign)
{
	int i,j,k;
	int start = numOfVariables-5;
	
	for (i=1;i<=3;i++)
		for (j=i+1;j<=4;j++)
			for (k=j+1;k<=5;k++)
				printf ("%d %d %d  0\n", sign*globalPermute[permute[i+start]], sign*globalPermute[permute[j+start]], sign*globalPermute[permute[k+start]]);
}



void mainUnsatGroup (int g, int sign)
{
	int i,j, k,v;
		
		for (i=1;i<=groupSize;i++)
		{
			for (j=1;j<=groupSize;j++)
				if (i != j) printf ("%d ", sign*globalPermute[permute[groupSize*g+j]]);
			printf ("0\n");
		}
	

}



void firstSatGroup (int g, int sign)
{
	int i,j;

	for (i=1;i<groupSize;i++)
		for (j=i+1;j<=groupSize;j++)
			printf ("%d %d 0\n", sign*globalPermute[permute[g*groupSize+i]], sign*globalPermute[permute[g*groupSize+j]]);
}



void swap (int i, int j)
{
	int temp;
	
	temp = permute[i];
	permute[i] = permute[j];
	permute[j] = temp;
}


void secondSatGroup(int l, int sign)
{
	int g,h;
	int v1, v2;
	
	v1 = 1;
	v2 = 1;
	for (g=0;g<groups;g++)
	{

		for (h=1;h<=groupSize;h++)
			printf ("%d ", sign*globalPermute[permute[g*groupSize+h]]);
		printf ("0\n");
	}
}

void initModel(int n)
{
	int v,g;
	
	model = (int *) malloc ((n+1)*sizeof(int));
	for (v=1; v<=n; v++)
		model[v] = -1;
		
	for (g=0;g<groups;g++)
	{
		v = g*groupSize + 1 + (myrand()%4);
		model[v] = 1;
	}
	
	
}

void initGlobalPermute()
{
	int v;
	globalPermute = (int *) malloc ((numOfVariables+1)*sizeof(int));

	if (reorder)
	{   
		setAnnealingPermute();
		for (v=1;v<=numOfVariables;v++)
			globalPermute[v] = permute[v];
	}
	else
	{
		setPlainPermute();
		for (v=1;v<=numOfVariables;v++)
			globalPermute[v] = permute[v];
	 }
	
}

void printModel (char *filename)
{
	FILE *f;
	int v, varsPerLine;
	
	f = fopen (filename, "w");
	if (f != NULL)
	{
		fprintf (f, "c Satisfiability model produced by sgen1\n");
		fprintf (f, "c %s\n", commandLine);
		fprintf (f, "s SATISFIABLE\n");
	
		varsPerLine = 20;
			
		for (v=1;v<=numOfVariables;v++)
		{
			if ( v % varsPerLine == 1)
				fprintf (f, "v ");
			fprintf (f, "%d ", globalPermute[v]*model[v]);
			if (v % varsPerLine == 0)
				fprintf (f, "\n");
		}
		if (numOfVariables % varsPerLine == 0)
			fprintf (f, "v ");
		fprintf (f, "0 \n");
		fclose (f);
	}
	else
	{
		fprintf (stderr, "Unable to write model to file <%s>\n", filename);
		exit(0);
	}
	
}

void minVariables ()
{
	int allClauses,numOfClauses, v,c, sign, deletedClause;

	 initModel(numOfVariables);
	 permute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
	 unpermute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
	 groupSize = 5;
	 initGlobalPermute();
	 setPlainPermute();
	allClauses =1;
	for (v=1;v<=numOfVariables;v++)
		allClauses *=2;
	if (isSat)
	{
		numOfClauses = allClauses - 1;
		deletedClause = (myrand() % numOfClauses);
		sign = 1;
		for (v=1;v<=numOfVariables;v++)
		{
			if (sign & deletedClause)
				model[v] = -1;
			else
				model[v] = 1;

			sign *= 2;
		}

	}
	else
	{
		numOfClauses = allClauses;
		deletedClause = -1;
	}
	printf ("p cnf %d %d\n", numOfVariables, numOfClauses);
	for (c=0;c<allClauses;c++)
		if (c != deletedClause)
		{
			sign = 1;
			for (v=1;v<=numOfVariables;v++)
			{
				if (sign & c)
					printf ("%d ",globalPermute[v]);
				else
				   printf ("%d ",-globalPermute[v]);
	
				sign *= 2;
			}
			printf ("0\n");
		}
   
	if (stringOptions[MODEL] != NULL)
		printModel (stringOptions[MODEL]);



}

void minLiterals ()
{
	int g,l;
	
	if (isSat)
	{
		groupSize = 5;
		numOfVariables = ((numOfVariables-1)/groupSize + 1) * groupSize;
		groups = numOfVariables/groupSize;
		initModel(numOfVariables);
		permute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
		unpermute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
		heading(numOfVariables);

		initGlobalPermute();
		setPlainPermute();
		for (g=0;g<groups;g++)
			firstSatGroup (g, -1);

		for (l=0;l<2;l++)
		{
			setAnnealingPermute1();
		   secondSatGroup(l, 1);
		}
		if (stringOptions[MODEL] != NULL)
			printModel (stringOptions[MODEL]);
	}
	else
	{
		groupSize = 4;
		if (numOfVariables == 1)
			numOfVariables = 2;
		groups = (numOfVariables+2)/groupSize;
		numOfVariables = groupSize*groups + 1;
		permute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
		unpermute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
		heading(numOfVariables);
		

		initGlobalPermute();
		setPlainPermute();
		for (g=0;g<groups-1;g++)
			mainUnsatGroup (g,-1);
		lastUnsatGroup (-1);
		setAnnealingPermute();
		for (g=0;g<groups-1;g++)
			mainUnsatGroup (g,1);
		lastUnsatGroup (1);
	}
}

int main(int argc, char **argv)
{

		
	getOptions (&argc, argv);
	
	checkOptions();
	
	numOfVariables = intOptions[NUM_VARIABLES];
	isSat = booleanOptions[SAT];
	reorder = booleanOptions[REORDER]; 

	mysrand (intOptions[RANDOM_SEED]);
	
	if (booleanOptions[MIN_VARIABLES])
	{
		minVariables();
		//minVariables (numOfVariables);
	}
	else
	{
		minLiterals();
		//minLiterals (numOfVariables);
	}
 

	
	return 1;
}
