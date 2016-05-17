/*
 * gui_e3.cpp
 *
 *  Created on: Aug 21, 2014
 *      Author: dimalit
 */

#include "gui_e4.h"

#include <gtkmm/builder.h>
#include <gtkmm/table.h>

#define UI_FILE_CONF "e4_config.glade"
#define UI_FILE_STATE "e4_state.glade"
#define UI_FILE_PETSC_SOLVER "e4_petsc_solver.glade"

E4ConfigWidget::E4ConfigWidget(const E4Config* cfg){
	if(cfg)
		this->config = new E4Config(*cfg);
	else
		this->config = new E4Config();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_CONF);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_N", entry_N);
	b->get_widget("entry_delta_0", entry_delta_0);

	b->get_widget("button_apply", button_apply);

	this->add(*root);

	config_to_widget();

	entry_N->signal_changed().connect(sigc::mem_fun(*this, &E4ConfigWidget::edit_anything_cb));
	entry_delta_0->signal_changed().connect(sigc::mem_fun(*this, &E4ConfigWidget::edit_anything_cb));

	button_apply->signal_clicked().connect(sigc::mem_fun(*this, &E4ConfigWidget::on_apply_cb));
}

void E4ConfigWidget::widget_to_config(){
	config->set_n(atoi(entry_N->get_text().c_str()));
	config->set_delta_0(atof(entry_delta_0->get_text().c_str()));

	button_apply->set_sensitive(false);
}
void E4ConfigWidget::config_to_widget(){
	std::ostringstream buf;
	buf << config->n();
	entry_N->set_text(buf.str());

	buf.str("");
	buf << config->delta_0();
	entry_delta_0->set_text(buf.str());

	button_apply->set_sensitive(false);
}

void E4ConfigWidget::edit_anything_cb(){
	button_apply->set_sensitive(true);
}

void E4ConfigWidget::on_apply_cb(){
	widget_to_config();
	m_signal_changed.emit();
}

void E4ConfigWidget::loadConfig(const OdeConfig* cfg){
	const E4Config* ecfg = dynamic_cast<const E4Config*>(cfg);
		assert(ecfg);
	delete this->config;
	this->config = new E4Config(*ecfg);
	config_to_widget();

	m_signal_changed.emit();
}

const OdeConfig* E4ConfigWidget::getConfig() {
	widget_to_config();
	return config;
}

E4StateGeneratorWidget::E4StateGeneratorWidget(const E4Config* _config){
	this->state = NULL;

	if(_config)
		this->config = new E4Config(*_config);
	else
		this->config = new E4Config();

	// TODO: may be state should remember its config?!

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_STATE);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_e", entry_e);
	b->get_widget("entry_phi", entry_phi);
	b->get_widget("entry_a0", entry_a0);


	b->get_widget("button_apply", button_apply);

	this->add(*root);

	entry_e->set_text("0.01");
	entry_phi->set_text("0.0");
	entry_a0->set_text("0.0");

	// signals
	entry_e->signal_changed().connect(sigc::mem_fun(*this, &E4StateGeneratorWidget::edit_anything_cb));
	entry_phi->signal_changed().connect(sigc::mem_fun(*this, &E4StateGeneratorWidget::edit_anything_cb));
	entry_a0->signal_changed().connect(sigc::mem_fun(*this, &E4StateGeneratorWidget::edit_anything_cb));

	button_apply->signal_clicked().connect(sigc::mem_fun(*this, &E4StateGeneratorWidget::on_apply_cb));

	assert(!this->state);
	newState();
}

E4StateGeneratorWidget::~E4StateGeneratorWidget(){
	// XXX where are deletes?
}

void E4StateGeneratorWidget::edit_anything_cb(){
	button_apply->set_sensitive(true);
}
void E4StateGeneratorWidget::on_apply_cb(){
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

	double e = atof(entry_e->get_text().c_str());
	double phi = atof(entry_phi->get_text().c_str());
	double a0 = atof(entry_a0->get_text().c_str());

	bool use_rand = false;
	double right = 2*M_PI;
	double left = 0.0;

	int N = config->n();

	double h = (right-left)/250;

	// particles 1
	for(int i=0; i<N; i++){
		double psi = rand() / (double)RAND_MAX * (right-left) + left;
		double z = i / (double)N * (right-left) + left;

		pb::E4State::Particles p;
		p.set_a(a0);
		p.set_a(psi);
		p.set_z(z);

		state->mutable_particles(i)->CopyFrom(p);
	}

	state->set_e(e);
	state->set_phi(phi);

	if(emit)
		m_signal_changed();
}

E4PetscSolverConfigWidget::E4PetscSolverConfigWidget(const E4PetscSolverConfig* config){
	if(config)
		this->config = new E4PetscSolverConfig(*config);
	else
		this->config = new E4PetscSolverConfig();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_PETSC_SOLVER);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_atol", entry_atol);
	b->get_widget("entry_rtol", entry_rtol);
	b->get_widget("entry_step", entry_step);
	adj_n_cores = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(b->get_object("adj_n_cores"));
	adj_n_cores->set_value(4);

	this->add(*root);

	config_to_widget();
}
const OdeSolverConfig* E4PetscSolverConfigWidget::getConfig(){
	widget_to_config();
	return config;
}
void E4PetscSolverConfigWidget::loadConfig(const OdeSolverConfig* config){
	const E4PetscSolverConfig* econfig = dynamic_cast<const E4PetscSolverConfig*>(config);
		assert(econfig);
	delete this->config;
	this->config = new E4PetscSolverConfig(*econfig);
	config_to_widget();
}

void E4PetscSolverConfigWidget::widget_to_config(){
	config->set_init_step(atof(entry_step->get_text().c_str()));
	config->set_atol(atof(entry_atol->get_text().c_str()));
	config->set_rtol(atof(entry_rtol->get_text().c_str()));
	config->set_n_cores(adj_n_cores->get_value());
}
void E4PetscSolverConfigWidget::config_to_widget(){
	std::ostringstream buf;
	buf << config->init_step();
	entry_step->set_text(buf.str());

	buf.str("");
	buf << config->atol();
	entry_atol->set_text(buf.str());

	buf.str("");
	buf << config->rtol();
	entry_rtol->set_text(buf.str());
	adj_n_cores->set_value(config->n_cores());
}
