/*
 * model_e3.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: dimalit
 */

#include "model_e3.h"

#include "rpc.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

E3State::E3State(const E3Config* config){
	int m = config->m();
	for(int i=0; i<m; i++){
		this->add_particles();
		this->mutable_particles(i)->set_a(1.0);
		this->mutable_particles(i)->set_ksi(0.0);
	}
	set_e(1.0);
	set_phi(0.0);
	set_simulated(false);
}

E3PetscSolver::E3PetscSolver(const E3PetscSolverConfig* scfg, const E3Config* pcfg, const E3State* init_state){
	time_passed = 0;
	steps_passed = 0;

	pconfig = new E3Config(*pcfg);
	sconfig = new E3PetscSolverConfig(*scfg);
	state = new E3State(*init_state);
	d_state = new E3State();
}

double E3PetscSolver::getTime() const {
	return time_passed;
}
double E3PetscSolver::getSteps() const {
	return steps_passed;
}

E3PetscSolver::~E3PetscSolver(){
	delete state;
	delete sconfig;
	delete pconfig;
}

// TODO: create universal base class for PETSc solvers - so not to copypaste!
// TODO: 1 universal code from TS solving?! (not to write it again and again!?)
void E3PetscSolver::run(int steps, double time){
	int rf, wf;
	pid_t child = rpc_call("../ts3/Debug/ts3", &rf, &wf);

//	int tmp = open("tmp", O_WRONLY | O_CREAT, 0664);
//	state->PrintDebugString();

	pb::E3Model all;
	all.mutable_sconfig()->CopyFrom(*sconfig);
	all.mutable_pconfig()->CopyFrom(*pconfig);
	all.mutable_state()->CopyFrom(*state);

	all.set_time(time);
	all.set_steps(steps);

//	int ftmp = open("all.tmp", O_CREAT | O_WRONLY);
	all.SerializeToFileDescriptor(wf);
//	close(ftmp);
	close(wf);		// need EOF for protobuf to catch the end of the message

//	close(tmp);

//	char buf;
//	while(read(rf, &buf, 1) > 0);
	waitpid(child, 0, 0);

	read(rf, &steps_passed, sizeof(steps_passed));
	read(rf, &time_passed, sizeof(time_passed));

	pb::E3Solution sol;
	sol.ParseFromFileDescriptor(rf);
	sol.mutable_state()->set_simulated(true);
	sol.mutable_d_state()->set_simulated(true);

	state->CopyFrom(sol.state());
	d_state->CopyFrom(sol.d_state());

	//sol.PrintDebugString();

	close(rf);
}
