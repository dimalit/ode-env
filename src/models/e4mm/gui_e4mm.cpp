/*
 * gui_e4.cpp
 *
 *  Created on: Jan 2, 2017
 *      Author: dimalit
 */

#include "gui_e4mm.h"

E4mmStateGeneratorWidget::E4mmStateGeneratorWidget(const E4mmConfig* _config){
	this->state = NULL;

	if(_config)
		this->config = new E4mmConfig(*_config);
	else
		this->config = new E4mmConfig();

	// TODO: may be state should remember its config?!
	pb::E4mmStateGeneratorConfig* sgc = new pb::E4mmStateGeneratorConfig();
	sgc->set_e0(0.1);
	sgc->set_a0(1.0);
	cfg_widget.setData(sgc);

	this->add(cfg_widget);
	cfg_widget.signal_changed().connect(sigc::mem_fun(*this, &E4mmStateGeneratorWidget::on_changed));

	assert(!this->state);
	newState();
}

E4mmStateGeneratorWidget::~E4mmStateGeneratorWidget(){
	// XXX where are deletes?
}

void E4mmStateGeneratorWidget::on_changed(){
	newState();
}

void E4mmStateGeneratorWidget::loadConfig(const OdeConfig* cfg){
	const E4mmConfig* ecfg = dynamic_cast<const E4mmConfig*>(cfg);
		assert(ecfg);

	delete this->config;
	this->config = new E4mmConfig(*ecfg);

	delete this->state;
	this->state = NULL;

	newState();
}
const OdeConfig* E4mmStateGeneratorWidget::getConfig(){
	return config;
}

const OdeState* E4mmStateGeneratorWidget::getState(){
	assert(config);
	assert(state);
	return state;
}

void E4mmStateGeneratorWidget::newState(bool emit){
	delete state;
	state = new E4mmState(config);

	const pb::E4mmStateGeneratorConfig* sgc = dynamic_cast<const pb::E4mmStateGeneratorConfig*>(cfg_widget.getData());

	double e0 = sgc->e0();
	double a0 = sgc->a0();

	double right = 2*M_PI;
	double left = 0.0;

	int N = config->n();

	for(int i=0; i<N; i++){
		double psi = rand() / (double)RAND_MAX * (right-left) + left;
		double z = i / (double)N * (right-left) + left;

		pb::E4mmState::Particles p;
		p.set_a(a0);
		p.set_psi(psi);
		p.set_z(z);

		state->mutable_particles(i)->CopyFrom(p);
	}

	double n0 = config->n0();
	double k = config->k();
	double s = config->s();
	for(int j=0; j<2*k+1; j++){
		double n = n0 - k + j;

		double e = e0/s/sqrt(M_PI)*exp( - (n-n0)*(n-n0) / (s*s) );
		double phi = rand()/(double)RAND_MAX*2*M_PI;
		state->mutable_fields(j)->set_e(e);
		state->mutable_fields(j)->set_phi(phi);
	}

	if(emit)
		m_signal_changed();
}

