/* constants */
#define NULL (void*) 0

#define AF_INET  0x02
#define AF_INET6 0x1C


/* structs */

typedef struct sock_addr_s {
	unsigned int _pad1;
	unsigned int _pad2;
	unsigned int addr;
	unsigned int _pad4;
	unsigned int _pad5;
	unsigned int _pad6;
	unsigned int _pad7;
	unsigned int _pad8;
} sock_addr_t;



/* stdlib functions */

void* memset(void* buf, int c, unsigned int n);
void* memcpy(void* dst, const void* src, int n);
void* malloc(unsigned int size);
int printf(char* str, ...);


/* network functions */

/* 
 * Create a new socket with IP protocol ip_proto on IP stack number ip_stack
 * _a and _b are unknown, use 1 and 6 respectively for IPv4 TCP.
 * Use 2 and 0x11 for UDP.
 * Returns NULL on failure and pointer to socket handle on success. 
 */
void* socket(int ip_proto, int _a, int _b, int ip_stack);

/* 
 * Close the give socket.
 */
void close(void* handle);

/* 
 * Bind the valid socket handle to the given address and port. 
 * Returns 0 on success. 
 */
int bind(void* handle, void* addr, int port);

/* 
 * Listen for a TCP connection on the bound socket. 
 * Arg _a is unknown, probably backlog like posix listen(). Use 1.
 * Returns 0 on success.
 */
int listen(void* handle, int _a);

/* 
 * Accept a connection on a listening socket. 
 * Args _a and _b are unknown, use 0 and 0 respectively.
 * Returns a pointer to a new socket handle on success or NULL on failure.
 */
void* accept(void* handle, int _a, int _b);

/* 
 * Send len bytes of data at the buffer buf on the connection handle.
 * Arg _a is unknown, use 0.
 * Returns the number of bytes send on success.
 */
int send(void* handle, char* buf, int len, int _a);

/* 
 * Read up to len bytes of data from the connection handle into buffer buf.
 * Arg _a is unknown, use 0.
 * Returns the number of bytes read on success.
 */
int recv(void* handle, char* buf, int len, int _a);

/*
 * For UDP sockets
 */
int recvfrom(void* handle, char* buf, int len, int flags, int* addr, int* addrlen);
int sendto(void* handle, char* buf, int len, int flags, int* addr, int addrlen);



/* transceiver functions */

/*
 * Set the freqency of the extra channel used by the spectrum analyzer
 */
void tune_aux_channel(double mhz_freq);


/* misc functions */

/*
 * Sleep for t milliseconds.
 */
void sleep(unsigned int t);

/*
 * Wait for t system ticks. Doesn't seem to cause a context switch. Mostly
 * used to let hardware settle after setting certain registers.
 */
void sleep2(unsigned int t);

/*
 * Create a thread with the given name, priority between 1 and 30(?) or 0 for
 * auto, stack size and a pointer to be filled with the thread id.
 */
int create_thread(char* name, int priority, unsigned int stack_size, int* id);

/*
 * Start a thread with the valid id returned from create_thread, running the
 * specifed function. Unknown how params work, use zero arg functions and pass
 * NULL for now.
 */
int start_thread(int id, void* function, void* params);

/*
 * Create a counting semaphore with the specified maximum and inital values
 * and name. _a is unknown, use 1. 
 */
int* sem_init(int _a, int max, int initial, char* name);

/*
 * Reduce the semaphore count by 1 or block if it is currently 0. Set block to
 * 1 to block forever or to 0 to block for a maximum of timeout seconds.
 */
int sem_wait(int* sem, int block, int timeout);

/*
 * Add one to the given semaphore.
 */
int sem_post(int* sem);

/*
 * Get the current count of the given semaphore.
 */
int sem_getcount(int* sem);
