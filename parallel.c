// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "log/log.h"
#include "utils.h"

#define NUM_THREADS		4

static int sum;
static os_graph_t *graph;
static os_threadpool_t *tp;
/* TODO: Define graph synchronization mechanisms. */
pthread_mutex_t graph_mutex;
pthread_mutex_t sum_mutex;

/* TODO: Define graph task argument. */
typedef struct task_args {
	os_node_t *node;
	unsigned int index;
} task_args;

static void compute_neighbours(void *arg)
{
	task_args *args = (task_args *) arg;

	pthread_mutex_lock(&sum_mutex);
	sum += args->node->info;
	graph->visited[args->index] = DONE;
	pthread_mutex_unlock(&sum_mutex);

	for (unsigned int i = 0; i < args->node->num_neighbours; i++) {
		pthread_mutex_lock(&graph_mutex);

		if (graph->visited[args->node->neighbours[i]] == NOT_VISITED) {
			graph->visited[args->node->neighbours[i]] = PROCESSING;
			pthread_mutex_unlock(&graph_mutex);
			task_args *new_args = malloc(sizeof(task_args));

			new_args->node = graph->nodes[args->node->neighbours[i]];
			new_args->index = args->node->neighbours[i];

			os_task_t *t = create_task(compute_neighbours, (void *) new_args, free);

			enqueue_task(tp, t);
		} else {
			pthread_mutex_unlock(&graph_mutex);
		}
	}
}

static void process_node(unsigned int idx)
{
	/* TODO: Implement thread-pool based processing of graph. */
	task_args *args = malloc(sizeof(task_args));

	args->node = graph->nodes[idx];
	args->index = idx;

	graph->visited[idx] = PROCESSING;
	os_task_t *t = create_task(compute_neighbours, (void *) args, free);

	enqueue_task(tp, t);
}

int main(int argc, char *argv[])
{
	FILE *input_file;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s input_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	input_file = fopen(argv[1], "r");
	DIE(input_file == NULL, "fopen");

	graph = create_graph_from_file(input_file);

	/* TODO: Initialize graph synchronization mechanisms. */
	int rc;

	rc = pthread_mutex_init(&graph_mutex, NULL);
	DIE(rc != 0, "pthread_mutex_init");
	rc = pthread_mutex_init(&sum_mutex, NULL);
	DIE(rc != 0, "pthread_mutex_init");

	tp = create_threadpool(NUM_THREADS);
	process_node(0);
	wait_for_completion(tp);
	destroy_threadpool(tp);

	pthread_mutex_destroy(&graph_mutex);
	pthread_mutex_destroy(&sum_mutex);

	printf("%d", sum);

	return 0;
}
