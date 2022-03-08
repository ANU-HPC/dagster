/* ggensata.c is a program to generate random SAT data            */
/* Modified Fri 17 Jan 09:59:21 AEDT 2020 by C. Gretton           */
/* Modified by Anbulagan based on gensata.c of Olivier Dubois     */
/* UTC-AI Lab,  February 4, 1996                                  */

/******************************************************************/
/*                         gensatandre.c                          */
/******************************************************************/
/*       Generation d'une donnee r-SAT aleatoire "in-abrege"      */
/******************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define or  ||
#define and &&

int N; /* valeur du nombre de variables */
int P; /* valeur du nombre de clauses */
int R; /* valeur de la longueur des clauses */ 

unsigned long int G; /* valeur du germe aleatoire */

int **SAT; /* matrice PxR des variables par clause */

int *REP;
int i,j,k;
char ch;

FILE *f;

unsigned long int RAND;
unsigned long int RANDMAX;
unsigned long int randomax;

/**************************************************************/
 unsigned long int private_random()
/**************************************************************/
{RAND=RAND*1103515245+12345;
 return (unsigned long int) RAND%RANDMAX;
}

/**************************************************************/
 void irandom(g) unsigned long int g;
/**************************************************************/
{RAND=g;
}

/**************************************************************/
 unsigned long int cr(n,p) int n,p;
/**************************************************************/
{int j; double c;

 if ((n>=p) && (p>=0)) {
  for (c=1.0,j=p-1;j>=0;j--) c=c*(n-j)/(p-j);
  return (unsigned long int)c; }
 else return 0;
}
  
/**************************************************************/
 unsigned long int powi(x,r) int x,r;
/**************************************************************/
{int i; unsigned long int pw;

 if (r<0) printf("Erreur : powi(x,r) avec r=%d\n",r);
 else {
  for (pw=1,i=1;i<=r;i++) pw=pw*x;
  return pw;
 }

 fprintf(stderr,"it seems the powi function is broken.\n");
 assert(0);
 exit(-1);
}

/**************************************************************/
void  geneREP_comb(n,r,kcr) int n,r; unsigned long int kcr;
/**************************************************************/
{int i,i1; char ok;
 
 for (i=1,i1=1;i<=r;i1++,i++)
  for (ok=0;ok==0;)
   if (kcr<cr(n-i1,r-i)) {
    REP[i-1]=i1; ok=1;
   } else {
    kcr=kcr-cr(n-i1,r-i); i1++;
   }
}

/**************************************************************/
void geneREP_inst(n,ir) int n; unsigned long int ir;
/**************************************************************/
{int j;

 for (j=1;j<=n;j++)
  if (ir<powi(2,n-j)) REP[j-1]=1;
  else {
   REP[j-1]=-1; ir=ir-powi(2,n-j);
  }
}
  
/**************************************************************/
int main(int argc, char * argv[6])
/**************************************************************/
{
 char outfile[15];

 if ( 6 != argc){
 /* arguments when execute ggens */ 
 /* N = nb. of variables         */
 /* P = nb. of clauses           */
 /* R = clause length            */
 /* G1 = germe begin -1          */
 /* G2 = germe end               */
   fprintf(stderr,
	   "USAGE: \n"				\
	   " 1. N = nb. of variables \n"	\
	   " 2. P = nb. of clauses \n"		\
	   " 3. R = clause length \n"					\
	   " 4. G1 = germe begin -1 -- i.e. in English, the starting index. Problems are indexed from G1 to G2 \n" \
	   " 5. G2 = germe end -- i.e. in English, the ending index \n");
 }
 

 int G1,G2;

 N = atoi(argv[1]);
 P = atoi(argv[2]);
 R = atoi(argv[3]);
 G1= atoi(argv[4]);
 G2= atoi(argv[5]);

 for (k=G1; k<G2; k++) {
    G = k+1;

    REP=(int *) malloc(R*sizeof(int));
    SAT=(int **) malloc(P*sizeof(long));
    for (i=0;i<P;i++) SAT[i]=(int *) malloc(R*sizeof(int));

    RANDMAX=cr(N,R);
    irandom(G);
    for (i=0;i<P;i++) {
      geneREP_comb(N,R,private_random());
      for (j=0;j<R;j++) SAT[i][j]=REP[j];
    }

    RANDMAX=32768; randomax=powi(2,R);
    irandom(G);
    for (i=0;i<P;i++) {
      geneREP_inst(R,private_random()*randomax/RANDMAX);
      for (j=0;j<R;j++) SAT[i][j]=SAT[i][j]*REP[j];
    }

    sprintf(outfile, "v%sc%sg%lu", argv[1], argv[2], G);

    f=fopen(outfile,"w");
    if (f==NULL) 
    printf("Erreur : ouverture du fichier \"%s\"\n","outfile");
    fprintf(f,"c germe %lu\n",G);
    fprintf(f,"p cnf %d %d\n",N,P);
    for (i=0;i<P;i++) {
      for (j=0;j<R;j++) fprintf(f," %3d",SAT[i][j]);
      fprintf(f," 0\n");
    }

    fclose(f);

    
    char dag_outfile[15 + 4];
    
    sprintf(dag_outfile, "v%sc%sg%lu.dag", argv[1], argv[2], G);

    f=fopen(dag_outfile,"w");
    if (f==NULL) 
      printf("Erreur : ouverture du fichier \"%s\"\n","dag_outfile");
    
    fprintf(f,"DAG-FILE\n");
    fprintf(f,"NODES:2\n");
    fprintf(f,"GRAPH:\n");
    fprintf(f,"0->1:1-5\n");
    fprintf(f,"CLAUSES:\n");
    fprintf(f,"0:%d-%d\n", 0,P-1-(int)ceil(0.06*(double)P)); // c.g. Magic number for now
    fprintf(f,"1:%d-%d\n", 0,P-1);
    fprintf(f,"REPORTING:\n");
    fprintf(f,"1-%d\n",N);

    fclose(f);
 }


 
 
 
 return 0;
}

