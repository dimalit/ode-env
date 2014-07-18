/*
 * model_e1.cpp
 *
 *  Created on: Jan 4, 2014
 *      Author: dimalit
 */

#include "model_e1.h"

#include <map>
#include <limits>
#include <algorithm>

#include <cstring>
#include <cstdio>

void E1State::sort_arrays(){
	// TODO: Do it better then copy many times:(

	// TODO: Хорошее задание студентам: эффективно отсортировать одновременно 2 массива через std::sort
	int* indices = new int[this->b_array.size()];
	int i = 0;
	std::for_each(indices, indices+b_array.size(), [&](int& v){v=i++;});
	struct comparator{
		double *arr;
		bool operator()(int a, int b){
			return arr[a] < arr[b];
		}
	};

	comparator c;
	c.arr = &this->ksi_array[0];
	std::sort(indices, indices+b_array.size(), c);

	// place them!
	double* tmp = new double[ksi_array.size()];
	for(unsigned i=0; i<ksi_array.size(); i++){
		tmp[i] = ksi_array[indices[i]];
	}
	std::copy(tmp, tmp+ksi_array.size(), ksi_array.begin());
	for(unsigned i=0; i<b_array.size(); i++){
		tmp[i] = b_array[indices[i]];
	}
	std::copy(tmp, tmp+b_array.size(), b_array.begin());
	delete[] tmp;

	delete[] indices;
}

E1PetscSolver::E1PetscSolver(const E1PetscSolverConfig* scfg, const E1Config* pcfg, const E1State* init_state){
	pconfig = new E1Config(*pcfg);
	sconfig = new E1PetscSolverConfig(*scfg);
	state = new E1State(*init_state);

	b = NULL;
	ksi = NULL;
	time_passed = 0.0;
	steps_passed = 0;
}
double E1PetscSolver::getTime() const {
	return time_passed;
}
double E1PetscSolver::getSteps() const {
	return 0.0;
}

E1PetscSolver::~E1PetscSolver(){
	delete state;
	delete sconfig;
	delete pconfig;
	delete b;
	delete ksi;
}

const OdeState* E1PetscSolver::run(int steps, double time){
//	PetscErrorCode ierr;
//	ierr = TSSolve(ts, NULL);assert(!ierr);
	// get all data
	int m = pconfig->g_m;
	double E = state->getE();
	double b = state->getB();
	double phi = state->getPhi();
	double ksi = state->getKsi();
	bool ksi_rand = state->getRandomKsi();
	bool ksi_linear = state->getLinearKsi();
	double tol = sconfig->g_tol;
	double step = sconfig->g_step;

	// check correctness
	assert(m > 0);
	assert(ksi >=0.0 && ksi <= 1.0);
	assert(tol > 0.0);
	assert(step > 0.0);

	// run!
	char buf_m[20];		sprintf(buf_m, 	"%d", m);
	char buf_E[20];		sprintf(buf_E,	"%f", E);
	char buf_b[20];		sprintf(buf_b,	"%f", b);
	char buf_phi[20];	sprintf(buf_phi,"%f", phi);

	char buf_ksi[20];
	if(ksi_rand)
		strcpy(buf_ksi, "rand");
	else if(ksi_linear)
		strcpy(buf_ksi, "linear");
	else
		sprintf(buf_ksi, "%f", ksi);

	char buf_tol[20];	sprintf(buf_tol, "%f", tol);
	char buf_step[20];	sprintf(buf_step,"%f", step);

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
		execl("../e1/Debug/e1", "e1", "-M", buf_m, "-E", buf_E, "-b", buf_b, "-phi", buf_phi, "-ksi", buf_ksi, "-tol", buf_tol, "-step", buf_step, "-input", (char*) NULL);
	}

	// send initial state
	int pipe_out = ends_to_child[1];
	close(ends_to_child[0]);
	FILE* wfd = fdopen(pipe_out, "wb");
	for(unsigned i=0; i<state->getBArray().size(); i++){
		fprintf(wfd, "%lf %lf ", state->getBArray()[i], state->getKsiArray()[i]);
	}
	fclose(wfd);

	// parent will receive
	int pipe_input = ends_from_child[0];
	close(ends_from_child[1]);
	FILE* fp = fdopen(pipe_input, "rb");

	delete[] this->b;
	delete[] this->ksi;
	this->b =  new double[m];
	this->ksi= new double[m];

	steps_passed = 0;
//	printf("t\tE\tphi\tb\tb_s\tksi\tksi_s\tmax\tmin\n");
	while(read_simulation(fp, m)==0)
		steps_passed++;
	printf("Iterations = %d\n", steps_passed);

	fclose(fp);	// close pipe input

	std::copy(this->ksi, this->ksi+pconfig->g_m, state->getKsiArray().begin());
	std::copy(this->b, this->b+pconfig->g_m, state->getBArray().begin());
	state->sort_arrays();

	return state;
}

int E1PetscSolver::read_simulation(FILE* fp, int m){
	double E, phi, t;

	int res;
	res = fscanf(fp, "%lf\tProcess [%d]", &t, &res);			// res for dummy
		if(res!=2){return -1;}

	res = fscanf(fp, "%lf %lf", &E, &phi);
		if(res!=2){return -1;}
	for(int i=0; i<m; i++){
		double d[2];
		res = fscanf(fp, "%lf %lf", &d[0], &d[1]);		// b and ksi
			if(res!=2){return -1;}
		b[i]   = d[0];
		ksi[i] = d[1];
	}
	// eat '\n' at end
	do{
		res = fgetc(fp);
	}while(res != EOF && res != '\n');

	// analyze
/*	double b_aver = average(b, m);
	double b_stddev = stddev(b, m);
	double center = weighted_average(ksi, b, m);
	double sigma  = weighted_stddev(ksi, b, m);
	double b_max = maximum(b, m);
	double b_min = minimum(b, m);
	printf("%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n", t, E, phi, b_aver, b_stddev, center, sigma, b_max, b_min);
*/

	state->setE(E);
	state->setPhi(phi);
	time_passed = t;

	return 0;
}
