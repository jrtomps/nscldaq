#ifndef CALCSTDAVG_H 
#define CALCSTDAVG_H

#include <stdlib.h>
#include <math.h>

typedef int (*qcompare_t)(const void *, const void *);

/*=====================================================================*/
static inline void calcstdavg(double v[],int cnt,double *a,double *s)
{
  int i;
  double tot,avg,wrk,stdev;

  tot = 0.0;
  for (i = 0; i < cnt; i++) tot += v[i];

  avg = tot/cnt;
  (*a) = avg;

  if (cnt > 1) {
    /* Calculate std. dev. */
    wrk = 0.0;        
    for (i = 0; i < cnt; i++) wrk += ((v[i] - avg)*(v[i] - avg));
    (*s) = sqrt(wrk / (cnt - 1));
  } else {
    (*s) = 0.0;
  }
}

/*=====================================================================*/
static inline int doublecmp(double *d1,double *d2)
{
  if ((*d1) < (*d2)) return(-1);
  else if ((*d1) > (*d2)) return(1);
  else return(0);
}

/*=====================================================================*/
static inline void calcmedian(double v[],int cnt,double *m)
{
  int i;
  int ccur,cwrk;
  double cur,wrk;
 
  qsort(v,cnt,sizeof(double),(qcompare_t)doublecmp);  

  cur = v[0];
  wrk = v[0];
  ccur = 1;
  cwrk = 1;

  for (i = 1; i < cnt; i++) {
    if (v[i] != wrk) {
      if (cwrk > ccur) {
        ccur = cwrk;
        cur = wrk;
      }

      wrk = v[i];
      cwrk = 0;
    } else {
      cwrk++;
    }
  }

  if (cwrk > ccur) cur = wrk;

  (*m) = cur;
}

#endif
