/*
 * rpc.cpp
 *
 *  Created on: Jul 10, 2014
 *      Author: dimalit
 */

#include <unistd.h>
#include <cassert>

void rpc_call(const char* cmd, int* rf, int* wf){
	int ends_to_child[2];
	int ends_from_child[2];
	assert(pipe(ends_to_child)==0);
	assert(pipe(ends_from_child)==0);

//	sprintf(cmd, "../e1/Debug/e1 -M %d -E %f -phi %f -b %f -ksi %s -tol %f -step %f", m, E, phi, b, buf_ksi, tol, step);
//	system(cmd);
	pid_t pid;
	pid = fork();
	if(pid == 0){		// child

		close(0);
		dup2(ends_to_child[0], 0);	// pipe read->stdin
		close(ends_to_child[0]);
		close(ends_to_child[1]);

		close(1);
		dup2(ends_from_child[1], 1);	// dup pipe write end to stdout
		close(ends_from_child[1]);
		close(ends_from_child[0]);

//		close(ends[0]);		// XXX Strange it gives SIGPIPE when uncommented...
		execl(cmd, (char*) NULL);
	}

	// will send initial state
	*rf = ends_to_child[1];
	close(ends_to_child[0]);

	// will receive results
	*wf = ends_from_child[0];
	close(ends_from_child[1]);
}
