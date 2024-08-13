#pragma once





AlifSizeT alifThread_getThreadID();





#ifdef USE_PTHREADS
#   include <pthread.h>
#   define NATIVE_TSS_KEY_T     PThreadKeyT
#elif defined(NT_THREADS)
/* In Windows, native TSS key type is DWORD,
   but hardcode the unsigned long to avoid errors for include directive.
*/
#   define NATIVE_TSS_KEY_T     unsigned long
#else
#   error "Require native threads."
#endif


#define ALIFTSS_NEEDS_INIT   {0}



