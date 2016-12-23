/*
 * model_e3.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: dimalit
 */

#include "model_e4.h"

#include <sstream>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include "rpc.h"

// XXX: whis should read like E4...if we use more than 1 specialization?
template<class SC, class PC, class S>
const char* EXPetscSolver<SC, PC, S>::ts_path = " ../ts4/Debug/ts4";

E4Config::E4Config(){
	set_n(1000);
	set_delta_0(0);
}

E4State::E4State(){
	set_e(0.001);
	set_phi(0.0);
}

E4State::E4State(const E4Config* config){
	int N = config->n();
	for(int i=0; i<N; i++){
		this->add_particles();
		this->mutable_particles(i)->set_a(1);
		this->mutable_particles(i)->set_psi(0.0);
		this->mutable_particles(i)->set_z(0.0);
		this->mutable_particles(i)->set_delta(0.0);
	}
	set_e(0.001);
	set_phi(0.0);
}

EXPetscSolverConfig::EXPetscSolverConfig(){
	set_atol(1e-6);
	set_rtol(1e-6);
	set_init_step(0.01);
	set_n_cores(1);
}

template<class SC, class PC, class S>
EXPetscSolver<SC,PC,S>::EXPetscSolver(const SConfig* scfg, const PConfig* pcfg, const State* init_state){
	time_passed = 0;
	steps_passed = 0;

	pconfig = new PConfig(*pcfg);
	sconfig = new SConfig(*scfg);
	state = new State(*init_state);
	d_state = new State();
}

template<class SC, class PC, class S>
EXPetscSolver<SC,PC,S>::~EXPetscSolver(){
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
template<class SC, class PC, class S>
void EXPetscSolver<SC,PC,S>::run(int steps, double time, bool use_step){
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
	cmd_stream << "mpiexec -n "<< n_cores << ts_path;// << " -info info.log";

	std::string cmd = cmd_stream.str();
	if(use_step)
		cmd += " use_step";
	int rfd, wfd;
	child = rpc_call(cmd.c_str(), &rfd, &wfd);
	rf = fdopen(rfd, "rb");
	wf = fdopen(wfd, "wb");

//	int tmp = open("tmp", O_WRONLY | O_CREAT, 0664);
//	state->PrintDebugString();

	pb::E4Model all;
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

template<class SC, class PC, class S>
bool EXPetscSolver<SC,PC,S>::step(){
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

template<class SC, class PC, class S>
bool EXPetscSolver<SC,PC,S>::read_results(){
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

	pb::E4Solution sol;
	extern void parse_with_prefix(google::protobuf::Message& msg, FILE* fp);
	parse_with_prefix(sol, rf);

//	sol.state().PrintDebugString();
//	fflush(stdout);

	state->CopyFrom(sol.state());
	d_state->CopyFrom(sol.d_state());
	return true;
}

template<class SC, class PC, class S>
void EXPetscSolver<SC,PC,S>::finish(){
	fputc('f', wf);
	fflush(wf);
}
