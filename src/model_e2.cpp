/*
 * model_e2.cpp
 *
 *  Created on: Jul 10, 2014
 *      Author: dimalit
 */

#include "model_e2.h"

#include "rpc.h"

#include <iostream>
#include <fcntl.h>
#include <sys/wait.h>

E2State::E2State(const E2Config* config){
	int n = config->n();
	for(int i=0; i<n; i++){
		this->add_particles();
	}
	set_simulated(false);
}

E2PetscSolver::E2PetscSolver(const E2PetscSolverConfig* scfg, const E2Config* pcfg, const E2State* init_state){
	pconfig = new E2Config(*pcfg);
	sconfig = new E2PetscSolverConfig(*scfg);
	state = new E2State(*init_state);
}

double E2PetscSolver::getTime() const {
	// TODO: stub
	return 0.0;
}

const OdeState* E2PetscSolver::getCurrentState() const{
	// TODO: stub
	return state;
}

E2PetscSolver::~E2PetscSolver(){
	delete state;
	delete sconfig;
	delete pconfig;
}

void E2PetscSolver::run(){
	int rf, wf;
	pid_t child = rpc_call("../ts/Debug/ts", &rf, &wf);

//	int tmp = open("tmp", O_WRONLY | O_CREAT, 0664);

	pb::E2Model all;
	all.mutable_sconfig()->CopyFrom(*sconfig);
	all.mutable_pconfig()->CopyFrom(*pconfig);
	all.mutable_state()->CopyFrom(*state);

	all.SerializeToFileDescriptor(wf);
	close(wf);		// need EOF for protobuf to catch the end of the message

//	close(tmp);

//	char buf;
//	while(read(rf, &buf, 1) > 0);
	waitpid(child, 0, 0);

	state->ParseFromFileDescriptor(rf);
	state->set_simulated(true);

	close(rf);
}
