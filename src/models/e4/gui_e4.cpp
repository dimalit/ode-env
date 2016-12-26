/*
 * gui_e3.cpp
 *
 *  Created on: Aug 21, 2014
 *      Author: dimalit
 */

#include "gui_e4.h"

E4StateGeneratorWidget::E4StateGeneratorWidget(const E4Config* _config){
	this->state = NULL;

	if(_config)
		this->config = new E4Config(*_config);
	else
		this->config = new E4Config();

	// TODO: may be state should remember its config?!
	pb::E4StateGeneratorConfig* sgc = new pb::E4StateGeneratorConfig();
	sgc->set_e0(0.01);
	sgc->set_phi0(0.0);
	sgc->set_a0(1.0);
	cfg_widget.setData(sgc);

	this->add(cfg_widget);
	cfg_widget.signal_changed().connect(sigc::mem_fun(*this, &E4StateGeneratorWidget::on_changed));

	assert(!this->state);
	newState();
}

E4StateGeneratorWidget::~E4StateGeneratorWidget(){
	// XXX where are deletes?
}

void E4StateGeneratorWidget::on_changed(){
	newState();
}

void E4StateGeneratorWidget::loadConfig(const OdeConfig* cfg){
	const E4Config* ecfg = dynamic_cast<const E4Config*>(cfg);
		assert(ecfg);

	delete this->config;
	this->config = new E4Config(*ecfg);

	delete this->state;
	this->state = NULL;

	newState();
}
const OdeConfig* E4StateGeneratorWidget::getConfig(){
	return config;
}

const OdeState* E4StateGeneratorWidget::getState(){
	assert(config);
	assert(state);
	return state;
}

void E4StateGeneratorWidget::newState(bool emit){
	delete state;
	state = new E4State(config);

	const pb::E4StateGeneratorConfig* sgc = dynamic_cast<const pb::E4StateGeneratorConfig*>(cfg_widget.getData());

	double e = sgc->e0();
	double phi = sgc->phi0();
	double a0 = sgc->a0();

	bool use_rand = false;
	double right = 2*M_PI;
	double left = 0.0;

	int N = config->n();

	double h = (right-left)/250;

	for(int i=0; i<N; i++){
		double psi = rand() / (double)RAND_MAX * (right-left) + left;
		double z = i / (double)N * (right-left) + left;
		double delta = rand() / (double)RAND_MAX * (2*config->delta_0()) - config->delta_0();

		pb::E4State::Particles p;
		p.set_a(a0);
		//p.set_a(a0*(1.0+0.2*sin(int(psi-z+phi))));
		p.set_psi(psi);
		p.set_z(z);
		p.set_delta(delta);

		state->mutable_particles(i)->CopyFrom(p);
	}

	state->set_e(e);
	state->set_phi(phi);

	if(emit)
		m_signal_changed();
}

