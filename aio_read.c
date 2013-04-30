#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<libaio.h>
#include<pthread.h>
#include<errno.h>

#define MAXEVENTS 100
#define PATH "Testfilenew.txt"	
#define BUFFERSIZE 4096

   /*
    *   Demonstrate asynchronous readss from a File using libaio
    *   Separate Threads are maintained for submitting and reaping IO requests
    *   Files should be opened using the O_DIRECT flag compulsorily
    *   For GNU compliant systems only 
    *   @Author : Akhil Rao
    */

io_context_t context;		/* Context of the application */
int filedes;			    /* file descriptor */
char* buffer;			    /* buffer for data excahnge */

void SubmitRead(int offset);
void *Reap(void *p);


int main(){
	int i;
	pthread_t reaperThread;
	/* memory alignment is very essential for memory writes*/
	posix_memalign((void**)&buffer,BUFFERSIZE,BUFFERSIZE);
	filedes=open(PATH,O_RDONLY|O_DIRECT,0644);	/* Open the file for reading */
	if(filedes<0){
		printf("Error Opening File\n");
		return 0; 
	}
	io_setup(MAXEVENTS,&context);				/* Initialize context */
    /* Create a separate thread to process results */
	pthread_create(&reaperThread,NULL,Reap,(void *)&i);

	for(i=0;i<100;i++){
		SubmitRead(i);					/* Submit Requests for Reading */
	}	

	pthread_join(reaperThread,NULL);	/* Wait till reaperThread Exits */

	/* Deallocate file descriptor and the context */

	close(filedes);
	io_destroy(context);
	return 0;
}
/* Submits the read IO request */
void SubmitRead(int offset){
	int x;
	struct iocb iocbr;
	struct iocb *iocbrp=&iocbr;
    /* Init iocb structure */
	io_prep_pread(&iocbr,filedes,buffer,BUFFERSIZE,BUFFERSIZE*offset);	
	iocbr.data=(void*)buffer;
	x=io_submit(context,1,&iocbrp);					/* Submit the request */
	if(x<0)
		printf("Error while submitting read request\n %d",x);
}
/* Reaps the outcome of IO requests submitted till now */
void *Reap(void *dummy){
	struct io_event list[MAXEVENTS];
	int events_completed=0,k=0,j,i;   /*
	char str[4097]={0},*data;	
	struct timespec timeout;					    /* timeout of 1 second */
	timeout.tv_sec=1;
	timeout.tv_nsec=0;
	printf("Starting Reaping Events");
	while(k<100){
        /* obtain the list of requests completed */
		events_completed=io_getevents(context,1,5,list,&timeout);	
		k+=events_completed;
		for(i=0;i<events_completed;i++){
			data=(char *)list[i].data;
			for(j=0;j<list[i].res;j++){
				str[j]=data[j];
			}		
			printf("%ld %s\n\n",list[i].res,str);	/*  Print the data read  */
		}	
	}
}
