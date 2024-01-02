#ifndef _WORKER_HEADER_
#define _WORKER_HEADER_

/**
 * @file Don't forget to link against '-lpthread'
*/

#include <stdlib.h>
#include <pthread.h>

typedef void *(*workerFunction_t)(void *);

typedef struct{
	pthread_t thread;
	workerFunction_t workerFunction;
	void *data;
	void *ret;
}worker_t;

void *workerWrapFunction(void *data){
	worker_t *worker = (worker_t*)data;
	worker->ret = worker->workerFunction(worker->data);
	return 0;
}

worker_t *workerCreate(workerFunction_t workerFunction, void *data){
	worker_t *worker = malloc(sizeof(worker_t));
	worker->data = data;
	worker->workerFunction = workerFunction;
	worker->ret = NULL;

	pthread_create(&(worker->thread), NULL, workerWrapFunction, worker);
	return worker;
}

void *workerWait(worker_t *worker){
	pthread_join(worker->thread, NULL);
	void *ret = worker->ret;
	free(worker);
	return ret;
}

#endif