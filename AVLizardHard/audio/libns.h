#ifndef _NS_LIBRARY_H_
#define _NS_LIBRARY_H_

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the processing
// Input
//      - mode  : input data buffer of the noised speech
// Return:
//			- 0 if OK, negative if fail

int ns_init(int mode);

// processing the noised speech
// Input
//      - pPcmAudio  : input data buffer of the noised speech
//											48KHz/2 Channels/16-bit PCM data
//      - nPcmLength : length of the data, amount of bytes
//
// Output:
//      - pPcmAudio : output data buffer of the denoised speech
// Return:
//			- 0 if OK, negative if fail

int ns_processing(char *pPcmAudio,const int nPcmLength);

// Uninitialize the processing
// Return:
//			- 0 if OK, negative if fail

int ns_uninit(void);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif	// _NS_LIBRARY_H_
