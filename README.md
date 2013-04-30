AIO-Test
========

Author: Akhil Rao

Project showing demonstrations of libaio for asynchronous IO in linux

This project shows how to use libaio for implementing asynchronous Disk IO.
Pthread is also used and hence to build the independent source files use the following:

gcc aio_read.c -lpthread -laio -o aio_read.o

gcc aio_write.c -lpthread -laio -o aio_write.o




