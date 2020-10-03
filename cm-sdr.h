#ifndef _CM_SDR_H
#define _CM_SDR_H

typedef union {
	char buf[sizeof(unsigned int)*2];
	struct {
		unsigned int _freq;
		unsigned int _dec;
	} __attribute__((packed)) cfg;
} cm_sdr_cfg;

#define cfg_freq cfg._freq
#define cfg_dec  cfg._dec


/*
 * Entrypoint
 */
int __start(void) __attribute__((section(".start")));

/*
 * Sampler thread
 */
int sampler(void);

/*
 * Network thread
 */
int net(void);


/*
 * Read samples while connected
 */
void read_data(void);

/*
 * Copy the samples from the start buffer into the dest buffer, packing the I
 * and Q values into one 4 byte word and decimating.
 */
unsigned int* compact(unsigned int* dest, unsigned int* src, unsigned int size);

#endif /* _CM_SDR_H */
