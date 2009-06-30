#ifndef XMLALLOC_H
#define XMLALLOC_H

#include <stdio.h>

#undef XMLALLOC_DEBUG

static inline void FREE(const char *eyecatch,void *p) { 
#ifdef XMLALLOC_DEBUG
  fprintf(stderr,"FREE(%s): %p\n",eyecatch,p); 
#endif
  if (p != NULL) free(p); 
  p = NULL;
}

static inline void *MALLOC(const char *eyecatch,size_t s) { 
  void *p;
  p = malloc(s);  
#ifdef XMLALLOC_DEBUG
  fprintf(stderr,"MALLOC(%s): %p to %d bytes\n",eyecatch,p,s); 
#endif
  return(p);
}

static inline void *REALLOC(const char *eyecatch,void *p,size_t s) { 
  void *origp = NULL,*newp = NULL;
  origp = p;
  newp = realloc(p,s); 
#ifdef XMLALLOC_DEBUG
  fprintf(stderr,"REALLOC(%s): %p --> %p to %d bytes\n",eyecatch,origp,newp,s); 
#endif
  return(newp);
}

static inline void *CALLOC(const char *eyecatch,size_t n,size_t s) { 
  void *p = NULL;
  p = calloc(n,s); 
#ifdef XMLALLOC_DEBUG
  fprintf(stderr,"CALLOC(%s): %p to %d*%d (%d) bytes\n",eyecatch,p,n,s,n*s); 
#endif
  return(p);
}

#endif
