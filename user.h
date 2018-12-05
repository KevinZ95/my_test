struct stat;
struct rtcdate;

// for module_1 struce

struct fss_request{
    char operation[7];
    int arg;
    char data[1024];
    int pid;
    
};


struct fss_response{
    int arg;
    char data[1024];
};


// system calls
int   fork(void);
int   exit(void) __attribute__((noreturn));
int   wait(void);
int   pipe(int *);
int   write(int, void *, int);
int   read(int, void *, int);
int   close(int);
int   kill(int);
int   exec(char *, char **);
int   open(char *, int);
int   mknod(char *, short, short);
int   unlink(char *);
int   fstat(int fd, struct stat *);
int   link(char *, char *);
int   mkdir(char *);
int   chdir(char *);
int   dup(int);
int   getpid(void);
char *sbrk(int);
int   sleep(int);
int   uptime(void);
// for module_2 system_call
char *shm_get(char *);
int   shm_rem(char *);

// for module_3 system_call
int   mux_create(char*);
void  mux_delete(int);
void  mux_lock(int);
void  mux_unlock(int);
// void  muxcv_wait(int);
// void  muxcv_signal(int);


// ulib.c
int   stat(char *, struct stat *);
char *strcpy(char *, char *);
void *memmove(void *, void *, int);
char *strchr(const char *, char c);
int   strcmp(const char *, const char *);
void  printf(int, char *, ...);
char *gets(char *, int max);
uint  strlen(char *);
void *memset(void *, int, uint);
void *malloc(uint);
void  free(void *);
int   atoi(const char *);
// for module_3 user_level functions
int   mutex_create(char*);
void  mutex_delete(int);
void  mutex_lock(int);
void  mutex_unlock(int);
void  cv_wait(int);
void  cv_signal(int);



//module 1 file system
int fss_read(int arg1);
int fss_write(int arg1, void * data);
int fss_open(int arg);
int fss_close(int arg);
int fss_mkdir(char * dir);
int faa_unlink(char * dir);


struct sharedMem{
    int status; //this is so important!!! set it equals to 0 if it is available and 1 if it is sending request.
    //we also need the response to the client, so, we define the status equals to 2 when the response is sent.
    struct fss_request request;
    struct fss_response response;
};


struct fss_response readResponse(struct fss_request* readRequest);
struct fss_response writeResponse(struct fss_request* writeRequest);
struct fss_response openResponse(struct fss_request* openRequest);
struct fss_response closeResponse(struct fss_request* closeRequest);
struct fss_response mkdirResopnse(struct fss_request* mkdirRequest);
struct fss_response unlinkresponse(struct fss_request* unlinkRequest);



