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
    *   Demonstrate asynchronous writes to a File using libaio
    *   Separate Threads are maintained for submitting and reaping IO requests
    *   Files should be opened using the O_DIRECT flag compulsorily
    *   For GNU compliant systems only 
    *   @Author : Akhil Rao
    */

io_context_t context=0;
int filedes,dummy;

char *buffer;
int num[100];

void errorvals();
void SubmitWrite(int counter);
void *Reap( void * p);

int main(){
	int i,j;
	pthread_t reaperThread;

    /* Open the file for writing */
	filedes=open(PATH,O_WRONLY|O_CREAT|O_DIRECT,0644);  
	if(filedes<=0)
	{
		printf("Error opening file\n");
		return 0;
	}
    /* Initialize Context */
	j=io_setup(MAXEVENTS,&context);
	/* Check for IO Request Completion in another thread */
	pthread_create(&reaperThread,NULL,Reap,(void *)&i);
	
	for(i=0;i<100;i++){
		/*** Memory alignment of buffer ***/
		posix_memalign((void**)&buffer,BUFFERSIZE,BUFFERSIZE);
		num[i]=i;
		for(j=0;j<BUFFERSIZE;j++){
            //Generate random alphabets to write to file*/
			buffer[j]=(char)(rand()%26+65);		
		}
		SubmitWrite(i);
	}
    /* Waiting for Reaping to Complete */
	pthread_join(reaperThread,NULL);
    /* Deallocate context and close the file */
    close(filedes);	
    io_destroy(context);
	
	return 0;
}
/* Submit request for write with offset */
void SubmitWrite(int offset){
	int x;
	struct iocb iocbw;
	struct iocb *iocbwp=&iocbw;	
    /* Initialize iocb for writing */
	io_prep_pwrite(&iocbw,filedes,buffer,BUFFERSIZE,offset*BUFFERSIZE);	
	iocbw.data=(void *)buffer;	
    /* Submit request to write */ 
	x=io_submit(context,1,&iocbwp);
	if(x<0)
		printf("Error in submitWrite() %d\n",x);
}
/* Responds to the IO completion Events */
void *Reap(void *p){
	struct io_event eventsReaped[MAXEVENTS];
	int event_completed,i,k=0;
	void *data;
	/* Timeout after 1 second */
	struct timespec timeout;
	timeout.tv_sec=1;
	timeout.tv_nsec=0;

	printf("Started Listening for reaping events\n");

	while(k<100){
		/* Obtain IO Completed events List */
		event_completed=io_getevents(context,1,5,eventsReaped,&timeout);
		k+=event_completed;
		for(i=0;i<event_completed;i++){
			data=eventsReaped[i].data;		
            /* Print the outcome of IO" */
            		free(data);
			printf("WriteCompleted %p  res=%ld\n",data,eventsReaped[i].res);
		}
	}

}
//Displays Error Values 
void errorvals(){
	printf("EINVAL=%d\n",EINVAL);
	printf("EBADF=%d\n",EBADF);
	printf("EFAULT=%d\n",EFAULT);
	printf("EAGAIN=%d\n",EAGAIN);
	printf("ENOSYS=%d\n",ENOSYS);
	
}

