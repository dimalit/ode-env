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

//	double right = 2*M_PI;
//	double left = 0.0;

	int N = config->n();

	// prepare shuffle
	int* shuffle = new int[N];
	for(int i=0; i<N; i++)shuffle[i] = i;
	std::random_shuffle(shuffle, shuffle+N);

	for(int i=0; i<N; i++){
		int j = shuffle[i];
		double z = i / (double)N * 5 + 0;

		//double psi = rand() / (double)RAND_MAX * (2*M_PI) + 0;
		double psi = ((j + (rand()/(double)RAND_MAX-0.5)) / N + z) * 2*M_PI;

		double delta = config->delta_0();//rand() / (double)RAND_MAX * (2*config->delta_0()) - config->delta_0();

		pb::E4State::Particles p;
		p.set_a(a0);//+0.1*sin(psi-2*M_PI*z));
		//p.set_a(a0*(1.0+0.2*sin(int(psi-z+phi))));
		p.set_psi(psi);
		p.set_z(z);
		p.set_delta(delta);

		state->mutable_particles(i)->CopyFrom(p);
	}

	delete shuffle;

	state->set_e(e);
	state->set_phi(phi);
	state->set_a0(a0);

	//center_masses();

	// move by dx,dy
	double dx = sgc->dx();
	double dy = sgc->dy();
	for(int i=0; i<N; i++){
		E4State::Particles& p = *state->mutable_particles(i);
		double a = p.a();

		double scalar_x = dx*cos(p.psi()-2*M_PI*p.z());
		double scalar_y = dy*sin(p.psi()-2*M_PI*p.z());
		p.set_a(a+scalar_x+scalar_y);
		scalar_x = -dx*sin(p.psi()-2*M_PI*p.z());
		scalar_y = +dy*cos(p.psi()-2*M_PI*p.z());
		p.set_psi(p.psi()+(scalar_x+scalar_y)/a);
	}

	if(emit)
		m_signal_changed();
}

void E4StateGeneratorWidget::center_masses(){
	int N = config->n();

	double sum_x = 0, sum_y = 0;

	for(int i=0; i<N; i++){
		E4State::Particles p = state->particles(i);

		sum_x += p.a()*cos(p.psi()-2*M_PI*p.z());
		sum_y += p.a()*sin(p.psi()-2*M_PI*p.z());
	}

	double dx = sum_x/N;
	double dy = sum_y/N;

	for(int i=0; i<N; i++){
		E4State::Particles& p = *state->mutable_particles(i);

		double scalar = dx*cos(p.psi()-2*M_PI*p.z()) + dy*sin(p.psi()-2*M_PI*p.z());
		p.set_a(p.a()-scalar);
		scalar = -dx*sin(p.psi()-2*M_PI*p.z()) + dy*cos(p.psi()-2*M_PI*p.z());
		p.set_psi(p.psi()-scalar/p.a());
	}
}
