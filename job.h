#ifndef JOB_H
#define JOB_H

#include "task.h"
#include "robot.h"

struct _robot;

typedef enum {
	JOB_NONE,
	JOB_GATHERER,
	JOB_MINER,
	JOB_COUNT,
} job_occupation;

typedef struct _job {
	const char *name;
	task* (*idle_task)(robot*);
	int mapchar;
} job;

void job_init();
const job* job_get(job_occupation job);


#endif

