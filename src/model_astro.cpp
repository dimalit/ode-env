/*
 * model_astro.cpp
 *
 *  Created on: Sep 18, 2016
 *      Author: dimalit
 */

#include "model_astro.h"

AstroConfig::AstroConfig() {
	this->set_theta(0);
	this->set_gamma(0);
}

AstroState::AstroState(const AstroConfig* cfg) {
	this->set_m(0);
	this->set_n_c(0);
	this->set_n_inc(0);
}

AstroSolverConfig::AstroSolverConfig(){
	this->set_step(0.01);
}

AstroSolver::AstroSolver(const AstroSolverConfig* scfg, const AstroConfig* pcfg,
		const AstroState* state) {
	this->sconfig = std::unique_ptr<AstroSolverConfig>(new AstroSolverConfig(*scfg));
	this->pconfig = std::unique_ptr<AstroConfig>(new AstroConfig(*pcfg));
	this->state = std::unique_ptr<AstroState>(new AstroState(*state));

	steps = 0;
	time = 0;
}

AstroSolver::~AstroSolver() {
}

void AstroSolver::run(int steps, double time, bool use_steps) {
}

bool AstroSolver::step() {
	// !!! just for tests
	double I0 = 0; // - make it real
	d_state->set_m( pconfig->gamma()*state->m() - 2*state->m()*state->n_c() + I0 );
	d_state->set_n_inc(1);
	d_state->set_n_c(0.2);

	double step = sconfig->step();
	state->set_m(step * d_state->m());
	state->set_n_inc(step * d_state->n_inc());
	state->set_n_c(step * d_state->n_c());
}

void AstroSolver::finish() {
}

double AstroSolver::getTime() const {
}

double AstroSolver::getSteps() const {
}
