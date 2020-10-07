#include "external.h"
#include "cm-sdr.h"

#define SAMPLE_SIZE   	8		/* bytes per sample */
#define BLOCK	 	1024*1024*2 	/* number of samples per buffer */
#define NUM_BUFS	3		/* number of buffers */

#define BIND_ADDR 	0xc0a86401	/* 192.168.100.1 */
#define PORT		1337		/* listen port */
#define IP_STACK	2

/* Globals */

/* holds frequency and downsampling factor */
cm_sdr_cfg cfg = {
	buf: {0}
};

int* buf[NUM_BUFS];	
int* r_sem[NUM_BUFS];	/* read ready */
int* w_sem[NUM_BUFS];	/* write ready */
int connected = 0;	/* connection status */
int r_index = 0;	/* current read (send) buffer */
int w_index = 0;	/* current write buffer */



int __start(void)
{
	int i;
	int res;
	int id = 0;

	/* initialize buffers */
	buf[0] = (int*) malloc(BLOCK * SAMPLE_SIZE * NUM_BUFS);
	memset(buf[0], 0, BLOCK * SAMPLE_SIZE * NUM_BUFS);
		
	for (i = 0; i < NUM_BUFS; i++)
	{
		if (i != 0)
			buf[i] = buf[i-1] + (BLOCK * SAMPLE_SIZE / sizeof(int));
		/* 
		 * Set write semaphore inital value to 1 so they are available
		 * immediately.
		 */
		r_sem[i] = sem_init(1, 1, 0, "cm-sdr-r");
		w_sem[i] = sem_init(1, 1, 1, "cm-sdr-w");
	}

	/* start the sampling and network threads */
	res = create_thread("cm-sdr", 22, 0x2000, &id);	
	res = start_thread(id, (void*)sampler, NULL);

	id = 0;
	res = create_thread("cm-sdr-net", 16, 0x2000, &id);	
	res = start_thread(id, (void*)net, NULL);

	return res;
}

int net(void)
{
	void* sock;
	void* conn;
	sock_addr_t sa = {0};
	int n = 1;
	int i;
	unsigned int sent = 0;
	int* end;
	unsigned int size;
	int recvd = 0;

	if (NULL == (sock = socket(AF_INET, 1, 6, IP_STACK)))
		return -1;

	sa.addr = BIND_ADDR;

	if (0 != bind(sock, &sa, PORT))
		return -2;

	if (0 != listen(sock, 1))
		return -3;

	while (1)
	{
		if (NULL == (conn = accept(sock, 0, 0)))
			return -4;
	
		recvd = 0;
		while (recvd < sizeof(cfg.buf))
		{
			i = recv(conn, cfg.buf + recvd, sizeof(cfg.buf) - recvd, 0);
			if (i <= 0)
			{
				close(conn);
				conn = NULL;
				break;
			}
			recvd += i;
		}

		if (!conn)
			continue;

		/* set tuner frequency */
		tune_aux_channel(double_div(int2double(cfg.cfg_freq), 1000000.0));
		sleep2(10);

		connected = 1;
		while(connected)
		{
			/* wait for data */
			sem_wait(r_sem[r_index], 1, 0);
			/* compact the data in place */
			end = compact(buf[r_index], buf[r_index], BLOCK);
	
			sent = 0;
			size = (end - buf[r_index]) * sizeof(unsigned int*);
			while (sent < size && n > 0) {
				n = send(conn, (char*)buf[r_index] + sent, size - sent, 0);
				sent += n;
			}
	
			/* buffer is ready to be written to again */
			sem_post(w_sem[r_index]);
			r_index = (r_index + 1) % NUM_BUFS;
	
			/* do this check last so the semaphores will be ready */
			if (n <= 0) {
				/* close socket and wait for new connection */
				connected = 0;
				n = 1;
				close(conn);
				conn = NULL;
			}
		}
	}
}

int sampler(void)
{
	/* wait for connection before reading */
	while (1)
	{
		while (!connected)
		{
			sleep(500);
		}
		read_data();
	}
	return 0;
}

void read_data(void)
{
	volatile unsigned int* t;
	t = (volatile unsigned int*)0xb3c1e858;
	*t &= 0xffff7fff;

	t = (volatile unsigned int*)0xb4e001e8;
	*t &= 0xfffc0fff;
	*t |= 0x00016000;

	/* buffer size in bytes */
	t = (volatile unsigned int*)0xb4e00170;
	*t = BLOCK * SAMPLE_SIZE;

	t = (volatile unsigned int*) 0xb4e0007c;
	*t = 0x003f0000;

	t = (volatile unsigned int*) 0xb4e0016c;
	*t = 0x00000006;

	t = (volatile unsigned int*) 0xb4e00278;
	*t |= 0x00000040;

	/* set adc to 14? bit mode */
	t = (volatile unsigned int*) 0xb3c10008;
	*t &= 0xffff0000;
	*t |= 0x00000008;
	/* *t |= 0x00000108; */

	while (connected)
	{
		/* wake up when writable */
		sem_wait(w_sem[w_index], 1, 0); 

		/* set destination buffer (physical address) */
		t = (volatile unsigned int*)0xb4e00178;
		*t = (volatile unsigned int)(buf[w_index]) & 0x1fffffff;

		/* set status bit */
		t = (volatile unsigned int*) 0xb4e0016c;
		*t |= 0x00000001;

		/* wait for completion */
		while (*t & 1)
		{
			sleep(2);
		}

		sem_post(r_sem[w_index]); /* indicate buffer is ready to read */
		w_index = (w_index + 1) % NUM_BUFS;
	}


	t = (volatile unsigned int*) 0xb3c1e858;
	*t |= 0x00008000;

	t = (volatile unsigned int*) 0xb4e00278;
	*t &= 0xffffffbf;
}

unsigned int* compact(unsigned int* dest, unsigned int* src, unsigned int size)
{
	unsigned int* p = dest;
	unsigned int* end;
	unsigned int* i = src;
	unsigned int* q = src + 1;

	/*
	 * If the 0x200000 bit is not set, the word is a Q value so move past
	 * it to the first I value and also drop the last word as well
	 * because it is a lone I value with no Q.
	 */
	if (!(src[0] & 0x200000))
	{
		i += 1;
		q += 1;
		src += 1;
		size -= 2;
	}


	end = src + (size * (SAMPLE_SIZE / sizeof(unsigned int)));
	while (q < end)
	{
		if ((*i & 0x3fff) > 0x2000) /* I is negative */
			*p = ((*i & 0x3fff) - 0x4000) << 16;
		else
			*p = (*i << 16);
		if ((*q & 0x3fff) > 0x2000) /* Q is negative */
			*p |= ((*q & 0x3fff) - 0x4000) & 0xffff;
		else
			*p |= (*q & 0xffff);
		p += 1;
		/* 
		 * skip samples to reduce sample rate, speed up this function,
		 * and decrese the amount of data to send back 
		 */
		i += 2 * cfg.cfg_downsample;
		q += 2 * cfg.cfg_downsample;
	}
	return p; 
}
