/*
 * gui_e3.cpp
 *
 *  Created on: Aug 21, 2014
 *      Author: dimalit
 */

#include "gui_e42m.h"

E42mStateGeneratorWidget::E42mStateGeneratorWidget(const E42mConfig* _config){
	this->state = NULL;

	if(_config)
		this->config = new E42mConfig(*_config);
	else
		this->config = new E42mConfig();

	// TODO: may be state should remember its config?!
	pb::E42mStateGeneratorConfig* sgc = new pb::E42mStateGeneratorConfig();
	sgc->set_e_p0(0.01);
	sgc->set_phi_p0(0.0);
	sgc->set_e_m0(0.01);
	sgc->set_phi_m0(0.0);
	sgc->set_a0(1.0);
	cfg_widget.setData(sgc);

	this->add(cfg_widget);
	cfg_widget.signal_changed().connect(sigc::mem_fun(*this, &E42mStateGeneratorWidget::on_changed));

	assert(!this->state);
	newState();
}

E42mStateGeneratorWidget::~E42mStateGeneratorWidget(){
	// XXX where are deletes?
}

void E42mStateGeneratorWidget::on_changed(){
	newState();
}

void E42mStateGeneratorWidget::loadConfig(const OdeConfig* cfg){
	const E42mConfig* ecfg = dynamic_cast<const E42mConfig*>(cfg);
		assert(ecfg);

	delete this->config;
	this->config = new E42mConfig(*ecfg);

	delete this->state;
	this->state = NULL;

	newState();
}
const OdeConfig* E42mStateGeneratorWidget::getConfig(){
	return config;
}

const OdeState* E42mStateGeneratorWidget::getState(){
	assert(config);
	assert(state);
	return state;
}

void E42mStateGeneratorWidget::newState(bool emit){
	delete state;
	state = new E42mState(config);

	const pb::E42mStateGeneratorConfig* sgc = dynamic_cast<const pb::E42mStateGeneratorConfig*>(cfg_widget.getData());

	double e_p = sgc->e_p0();
	double phi_p = sgc->phi_p0();
	double e_m = sgc->e_m0();
	double phi_m = sgc->phi_m0();
	double a0 = sgc->a0();

//	double right = 2*M_PI;
//	double left = 0.0;

	int N = config->n();

	for(int i=0; i<N; i++){
		double psi = rand() / (double)RAND_MAX * (2*M_PI) + 0;
		double z = i / (double)N * 5 + 0;
		double delta = rand() / (double)RAND_MAX * (2*config->delta_0()) - config->delta_0();

		pb::E42mState::Particles p;
		p.set_a(a0);//+0.1*sin(psi-2*M_PI*z));
		//p.set_a(a0*(1.0+0.2*sin(int(psi-z+phi))));
		p.set_psi(psi);
		p.set_z(z);
		p.set_delta(delta);

		state->mutable_particles(i)->CopyFrom(p);
	}

	state->set_e_p(e_p);
	state->set_phi_p(phi_p);
	state->set_e_m(e_m);
	state->set_phi_m(phi_m);

//	center_masses();

	if(emit)
		m_signal_changed();
}

void E42mStateGeneratorWidget::center_masses(){
	int N = config->n();

	double sum_x = 0, sum_y = 0;

	for(int i=0; i<N; i++){
		E42mState::Particles p = state->particles(i);

		sum_x += p.a()*cos(p.psi()-2*M_PI*p.z());
		sum_y += p.a()*sin(p.psi()-2*M_PI*p.z());
	}

	double dx = sum_x/N;
	double dy = sum_y/N;

	for(int i=0; i<N; i++){
		E42mState::Particles& p = *state->mutable_particles(i);

		double scalar = dx*cos(p.psi()-2*M_PI*p.z()) + dy*sin(p.psi()-2*M_PI*p.z());
		p.set_a(p.a()-scalar);
		scalar = -dx*sin(p.psi()-2*M_PI*p.z()) + dy*cos(p.psi()-2*M_PI*p.z());
		p.set_psi(p.psi()-scalar/p.a());
	}
}
