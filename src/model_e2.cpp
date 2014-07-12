/*
 * model_e2.cpp
 *
 *  Created on: Jul 10, 2014
 *      Author: dimalit
 */

#include "model_e2.h"

#include "rpc.h"

E2State::E2State(const E2Config* config){
	int n = config->n();
	for(int i=0; i<n; i++){
		this->add_particles();
	}
}

E2InstanceFactory::E2InstanceFactory(){
	OdeInstanceFactoryManager::getInstance()->add(this);
}

E2PetscSolver::E2PetscSolver(const E2Config* pcfg, const E2State* init_state, const E2PetscSolverConfig* scfg){
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
	rpc_call("../ts/Debug", &rf, &wf);

	pconfig->SerializeToFileDescriptor(wf);
	state->SerializePartialToFileDescriptor(wf);
	sconfig->SerializePartialToFileDescriptor(wf);

	close(wf);
	close(rf);
}
