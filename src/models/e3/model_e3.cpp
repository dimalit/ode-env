/*
 * model_e3.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: dimalit
 */

#include "model_e3.h"

#include <sstream>

#include <rpc.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

void parse_with_prefix(google::protobuf::Message& msg, FILE* fp){
	int size;
	int ok = fread(&size, sizeof(size), 1, fp);
	assert(ok == 1);

	//TODO:without buffer cannot read later bytes
	char *buf = (char*)malloc(size);
	ok = fread(buf, 1, size, fp);
		assert(ok==size);
	msg.ParseFromArray(buf, size);
	free(buf);
}

E3Config::E3Config(){
	set_m(1000);
	set_n(1.0);
	set_theta_e(0);
	set_gamma_0_2(0.0);
	set_delta_e(0);
	set_r_e(0.0);
	set_alpha(0.0);
}

E3State::E3State(){
	set_e(0.001);
	set_phi(0.0);
	set_simulated(false);
}

E3State::E3State(const E3Config* config){
	int m = config->m();
	for(int i=0; i<m; i++){
		this->add_particles();
		this->mutable_particles(i)->set_a(1);
		this->mutable_particles(i)->set_ksi(0.0);
	}
	set_e(0.001);
	set_phi(0.0);
	set_simulated(false);
}

E3PetscSolverConfig::E3PetscSolverConfig(){
	set_atol(1e-6);
	set_rtol(1e-6);
	set_init_step(0.01);
	set_solver(E3PetscSolverConfig::rhs);
	set_model("tm");
	set_n_cores(1);
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
	if(wf){
		fputc('f', wf);
		fflush(wf);
	}
	delete state;
	delete sconfig;
	delete pconfig;
}

// TODO: create universal base class for PETSc solvers - so not to copypaste!
// TODO: 1 universal code from TS solving?! (not to write it again and again!?)
void E3PetscSolver::run(int steps, double time, bool use_step){
//	printf("run started\n");
//	fflush(stdout);

	static int run_cnt = 0;
	run_cnt++;

	int n_cores = 1;
	if(this->sconfig->has_n_cores())
		n_cores = this->sconfig->n_cores();
	std::ostringstream cmd_stream;
//	cmd_stream << "mpiexec -n "<< n_cores << " --host 192.168.0.101 ./Debug/ts3";
//	cmd_stream << "mpiexec -n "<< n_cores << " --host 10.0.0.205 /home/dimalit/workspace/ts3/Debug/ts3";
	cmd_stream << "mpiexec -n "<< n_cores << " ../ts3/Debug/ts3";// << " -info info.log";

	std::string cmd = cmd_stream.str();
	if(use_step)
		cmd += " use_step";
	int rfd, wfd;
	child = rpc_call(cmd.c_str(), &rfd, &wfd);
	rf = fdopen(rfd, "rb");
	wf = fdopen(wfd, "wb");

//	int tmp = open("tmp", O_WRONLY | O_CREAT, 0664);
//	state->PrintDebugString();

	pb::E3Model all;
	all.mutable_sconfig()->CopyFrom(*sconfig);
	all.mutable_pconfig()->CopyFrom(*pconfig);
	all.mutable_state()->CopyFrom(*state);

	int size = all.ByteSize();

	int ok;
	ok = fwrite(&size, sizeof(size), 1, wf);
		assert(ok == 1);

	fflush(wf);
	all.SerializeToFileDescriptor(fileno(wf));

	ok = fwrite(&steps, sizeof(steps), 1, wf);
		assert(ok == 1);
	ok = fwrite(&time, sizeof(time), 1, wf);
		assert(ok == 1);
	fflush(wf);

	if(!use_step){		// just final step
		bool res = read_results();
			assert(res==true);		// last
		waitpid(child, 0, 0);
		fclose(rf); rf = NULL;
		fclose(wf); wf = NULL;
	}
}

bool E3PetscSolver::step(){
	if(waitpid(child, 0, WNOHANG)!=0){
		fclose(rf); rf = NULL;
		fclose(wf); wf = NULL;
		return false;
	}

	fputc('s', wf);
	fflush(wf);

	if(!read_results()){
		// TODO: will it ever run? (see waitpid above)
		waitpid(child, 0, 0);		// was before read - here for tests
		fclose(rf); rf = NULL;
		fclose(wf); wf = NULL;
		return false;
	}
	return true;
}

bool E3PetscSolver::read_results(){
	int ok;
	ok = fread(&steps_passed, sizeof(steps_passed), 1, rf);
	// TODO: read=0 no longer works with mpiexec (process isn't zombie)
	if(ok==0)
		return false;
	else
		assert(ok == 1);
	ok = fread(&time_passed, sizeof(time_passed), 1, rf);
		assert(ok == 1);

//	printf("%d %lf %s\n", steps_passed, time_passed, sconfig->model().c_str());
//	fflush(stdout);

	pb::E3Solution sol;
	parse_with_prefix(sol, rf);
	sol.mutable_state()->set_simulated(true);
	sol.mutable_d_state()->set_simulated(true);

//	sol.state().PrintDebugString();
//	fflush(stdout);

	state->CopyFrom(sol.state());
	d_state->CopyFrom(sol.d_state());
	return true;
}

void E3PetscSolver::finish(){
	fputc('f', wf);
	fflush(wf);
}
