/*
 * model_e2.cpp
 *
 *  Created on: Jul 10, 2014
 *      Author: dimalit
 */

#include "model_e2.h"

#include <rpc.h>

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

E2State::E2State(const E2Config* config){
	int n = config->n();
	for(int i=0; i<n; i++){
		this->add_particles();
	}
	set_simulated(false);
}

E2PetscSolver::E2PetscSolver(const E2PetscSolverConfig* scfg, const E2Config* pcfg, const E2State* init_state){
	time_passed = 0;
	steps_passed = 0;

	pconfig = new E2Config(*pcfg);
	sconfig = new E2PetscSolverConfig(*scfg);
	state = new E2State(*init_state);
	d_state = new E2State();
}

double E2PetscSolver::getTime() const {
	return time_passed;
}
double E2PetscSolver::getSteps() const {
	return steps_passed;
}

E2PetscSolver::~E2PetscSolver(){
	delete state;
	delete sconfig;
	delete pconfig;
}

void E2PetscSolver::run(int steps, double time, bool use_step){
	int rf, wf;
	pid_t child = rpc_call("../ts/Debug/ts", &rf, &wf);

//	int tmp = open("tmp", O_WRONLY | O_CREAT, 0664);

	pb::E2Model all;
	all.mutable_sconfig()->CopyFrom(*sconfig);
	all.mutable_pconfig()->CopyFrom(*pconfig);
	all.mutable_state()->CopyFrom(*state);

	all.set_time(time);
	all.set_steps(steps);

	all.SerializeToFileDescriptor(wf);
	close(wf);		// need EOF for protobuf to catch the end of the message

//	close(tmp);

//	char buf;
//	while(read(rf, &buf, 1) > 0);
	waitpid(child, 0, 0);

	read(rf, &steps_passed, sizeof(steps_passed));
	read(rf, &time_passed, sizeof(time_passed));

	state->ParseFromFileDescriptor(rf);
	state->set_simulated(true);

	close(rf);
}
