#ifndef _LIBNS_H__
#define _LIBNS_H__
extern int ns_init(void);
extern int ns_processing(char *pPcmAudio,const int nPcmLength);
extern int ns_uninit(void);
#endif
