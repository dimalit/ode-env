/*
 * gui_e3.cpp
 *
 *  Created on: Aug 21, 2014
 *      Author: dimalit
 */

#include "gui_e3.h"

#include <gtkmm/builder.h>
#include <gtkmm/table.h>

#define UI_FILE_CONF "e3_config.glade"
#define UI_FILE_STATE "e3_state.glade"
#define UI_FILE_PETSC_SOLVER "e3_petsc_solver.glade"

E3ConfigWidget::E3ConfigWidget(const E3Config* cfg){
	if(cfg)
		this->config = new E3Config(*cfg);
	else
		this->config = new E3Config();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_CONF);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_m", entry_m);
	b->get_widget("entry_n", entry_n);
	b->get_widget("entry_theta", entry_theta);
	b->get_widget("entry_gamma", entry_gamma);
	b->get_widget("entry_delta", entry_delta);
	b->get_widget("entry_r", entry_r);

	b->get_widget("button_apply", button_apply);

	this->add(*root);

	config_to_widget();

	entry_m->signal_changed().connect(sigc::mem_fun(*this, &E3ConfigWidget::edit_anything_cb));
	entry_n->signal_changed().connect(sigc::mem_fun(*this, &E3ConfigWidget::edit_anything_cb));
	entry_theta->signal_changed().connect(sigc::mem_fun(*this, &E3ConfigWidget::edit_anything_cb));
	entry_gamma->signal_changed().connect(sigc::mem_fun(*this, &E3ConfigWidget::edit_anything_cb));
	entry_delta->signal_changed().connect(sigc::mem_fun(*this, &E3ConfigWidget::edit_anything_cb));
	entry_r->signal_changed().connect(sigc::mem_fun(*this, &E3ConfigWidget::edit_anything_cb));

	button_apply->signal_clicked().connect(sigc::mem_fun(*this, &E3ConfigWidget::on_apply_cb));
}

void E3ConfigWidget::widget_to_config(){
	config->set_m(atoi(entry_m->get_text().c_str()));
	config->set_n(atof(entry_n->get_text().c_str()));
	config->set_theta_e(atof(entry_theta->get_text().c_str()));
	config->set_delta_e(atof(entry_delta->get_text().c_str()));
	config->set_gamma_0_2(atof(entry_gamma->get_text().c_str()));
	config->set_r_e(atof(entry_r->get_text().c_str()));

	button_apply->set_sensitive(false);
}
void E3ConfigWidget::config_to_widget(){
	std::ostringstream buf;
	buf << config->m();
	entry_m->set_text(buf.str());

	buf.str("");
	buf << config->n();
	entry_n->set_text(buf.str());

	buf.str("");
	buf << config->theta_e();
	entry_theta->set_text(buf.str());

	buf.str("");
	buf << config->gamma_0_2();
	entry_gamma->set_text(buf.str());

	buf.str("");
	buf << config->delta_e();
	entry_delta->set_text(buf.str());

	buf.str("");
	buf << config->r_e();
	entry_r->set_text(buf.str());

	button_apply->set_sensitive(false);
}

void E3ConfigWidget::edit_anything_cb(){
	button_apply->set_sensitive(true);
}

void E3ConfigWidget::on_apply_cb(){
	widget_to_config();
	m_signal_changed.emit();
}

void E3ConfigWidget::loadConfig(const OdeConfig* cfg){
	const E3Config* ecfg = dynamic_cast<const E3Config*>(cfg);
		assert(ecfg);
	delete this->config;
	this->config = new E3Config(*ecfg);
	config_to_widget();

	m_signal_changed.emit();
}

const OdeConfig* E3ConfigWidget::getConfig() {
	widget_to_config();
	return config;
}

E3StateGeneratorWidget::E3StateGeneratorWidget(const E3Config* _config){
	this->state = NULL;

	if(_config)
		this->config = new E3Config(*_config);
	else
		this->config = new E3Config();

	// TODO: may be state should remember its config?!

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_STATE);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_e", entry_e);
	b->get_widget("entry_phi", entry_phi);
	b->get_widget("entry_a", entry_a);
	b->get_widget("entry_a2", entry_a2);

	b->get_widget("entry_eta", entry_eta);
	b->get_widget("entry_eta2", entry_eta2);
	b->get_widget("entry_n1", entry_n1);

	b->get_widget("button_apply", button_apply);

	this->add(*root);

	entry_e->set_text("0.01");
	entry_phi->set_text("0.0");
	entry_a->set_text("1.0");
	entry_a2->set_text("0.1");
	entry_eta->set_text("0.0");
	entry_eta2->set_text("0.0");
	entry_n1->set_text("1.0");

	// signals
	entry_e->signal_changed().connect(sigc::mem_fun(*this, &E3StateGeneratorWidget::edit_anything_cb));
	entry_phi->signal_changed().connect(sigc::mem_fun(*this, &E3StateGeneratorWidget::edit_anything_cb));
	entry_a->signal_changed().connect(sigc::mem_fun(*this, &E3StateGeneratorWidget::edit_anything_cb));
	entry_a2->signal_changed().connect(sigc::mem_fun(*this, &E3StateGeneratorWidget::edit_anything_cb));
	entry_eta->signal_changed().connect(sigc::mem_fun(*this, &E3StateGeneratorWidget::edit_anything_cb));
	entry_eta2->signal_changed().connect(sigc::mem_fun(*this, &E3StateGeneratorWidget::edit_anything_cb));
	entry_n1->signal_changed().connect(sigc::mem_fun(*this, &E3StateGeneratorWidget::edit_anything_cb));

	button_apply->signal_clicked().connect(sigc::mem_fun(*this, &E3StateGeneratorWidget::on_apply_cb));

	assert(!this->state);
	newState();
}

E3StateGeneratorWidget::~E3StateGeneratorWidget(){
	// XXX where are deletes?
}

void E3StateGeneratorWidget::edit_anything_cb(){
	button_apply->set_sensitive(true);
}
void E3StateGeneratorWidget::on_apply_cb(){
	newState();
}


void E3StateGeneratorWidget::loadConfig(const OdeConfig* cfg){
	const E3Config* ecfg = dynamic_cast<const E3Config*>(cfg);
		assert(ecfg);

	delete this->config;
	this->config = new E3Config(*ecfg);

	delete this->state;
	this->state = NULL;

	newState();
}
const OdeConfig* E3StateGeneratorWidget::getConfig(){
	return config;
}

const OdeState* E3StateGeneratorWidget::getState(){
	assert(config);
	assert(state);
	return state;
}

double rand1(){
	return (double)rand()/RAND_MAX;
}

void E3StateGeneratorWidget::newState(bool emit){
	delete state;
	state = new E3State(config);

	double e = atof(entry_e->get_text().c_str());
	double phi = atof(entry_phi->get_text().c_str());
	double a = atof(entry_a->get_text().c_str());
	double a2 = atof(entry_a2->get_text().c_str());
	double eta1 = atof(entry_eta->get_text().c_str());
	double eta2 = atof(entry_eta2->get_text().c_str());
	double n1 = atof(entry_n1->get_text().c_str());

	bool use_rand = false;
	double right = 0.5;
	double left = -0.5;

	int m = config->m();
	int m1 = m*n1 + 0.5;
	int m2 = m - m1;

	double h = (right-left)/240;
	double h2 = (eta2-eta1)/m1;

	// prepare shuffle
	int* shuffle = new int[m1];
	for(int i=0; i<m1; i++)shuffle[i] = i;
	std::random_shuffle(shuffle, shuffle+m1);

	// particles 1
	for(int i=0; i<m1; i++){
		double ksi;
		if(use_rand)
			ksi = rand() / (double)RAND_MAX * (right-left) + left;
		else
			ksi = i / (double)m1 * (right-left) + left + /*rand*/ (rand1()-0.5)*h;

		pb::E3State::Particles p;
		p.set_ksi(ksi);

		p.set_a(a);
		int j = shuffle[i];
		double eta = j / (double)m1 * (eta2-eta1) + eta1 + (rand1()-0.5)*h2;	// eta1 and eta2 are now borders
		p.set_eta(eta);

		state->mutable_particles(i)->CopyFrom(p);
	}

	// particles 2
	for(int i=0; i<m2; i++){
		double ksi;
		if(use_rand)
			ksi = rand() / (double)RAND_MAX * (right-left) + left;
		else
			ksi = i / (double)m2 * (right-left) + left + /*rand*/ (rand1()-0.5)*h;

		pb::E3State::Particles p;
		p.set_ksi(ksi);

		p.set_a(a2);
		p.set_eta(0.0);

		state->mutable_particles(m1+i)->CopyFrom(p);
	}

	state->set_e(e);
	state->set_phi(phi);
	state->set_a0(a);
	state->set_simulated(false);

	if(emit)
		m_signal_changed();
}

E3PetscSolverConfigWidget::E3PetscSolverConfigWidget(const E3PetscSolverConfig* config){
	if(config)
		this->config = new E3PetscSolverConfig(*config);
	else
		this->config = new E3PetscSolverConfig();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_PETSC_SOLVER);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_atol", entry_atol);
	b->get_widget("entry_rtol", entry_rtol);
	b->get_widget("entry_step", entry_step);
	adj_n_cores = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(b->get_object("adj_n_cores"));
	adj_n_cores->set_value(4);

	combo_type = Gtk::manage( new Gtk::ComboBoxText() );
	combo_type->append("te");
	combo_type->append("tm");
	(dynamic_cast<Gtk::Table*>(root))->attach(*combo_type, 1, 2, 3, 4);

	combo_solver = Gtk::manage( new Gtk::ComboBoxText() );
	for(int i=E3PetscSolverConfig::Solver_MIN; i<=E3PetscSolverConfig::Solver_MAX; ++i){
		std::string name = E3PetscSolverConfig::Solver_Name((E3PetscSolverConfig::Solver)i);
		combo_solver->append(name);
	}
	(dynamic_cast<Gtk::Table*>(root))->attach(*combo_solver, 1, 2, 4, 5);

	this->add(*root);

	config_to_widget();
}
const OdeSolverConfig* E3PetscSolverConfigWidget::getConfig(){
	widget_to_config();
	return config;
}
void E3PetscSolverConfigWidget::loadConfig(const OdeSolverConfig* config){
	const E3PetscSolverConfig* econfig = dynamic_cast<const E3PetscSolverConfig*>(config);
		assert(econfig);
	delete this->config;
	this->config = new E3PetscSolverConfig(*econfig);
	config_to_widget();
}

void E3PetscSolverConfigWidget::widget_to_config(){
	config->set_init_step(atof(entry_step->get_text().c_str()));
	config->set_atol(atof(entry_atol->get_text().c_str()));
	config->set_rtol(atof(entry_rtol->get_text().c_str()));
	config->set_model(combo_type->get_active_text());
	config->set_n_cores(adj_n_cores->get_value());

	std::string solver_name = combo_solver->get_active_text();
	E3PetscSolverConfig::Solver s_enum;
	bool ok = E3PetscSolverConfig::Solver_Parse(solver_name, &s_enum);
	assert(ok);
	config->set_solver(s_enum);
}
void E3PetscSolverConfigWidget::config_to_widget(){
	std::ostringstream buf;
	buf << config->init_step();
	entry_step->set_text(buf.str());

	buf.str("");
	buf << config->atol();
	entry_atol->set_text(buf.str());

	buf.str("");
	buf << config->rtol();
	entry_rtol->set_text(buf.str());

	combo_type->set_active_text(config->model());
	combo_solver->set_active_text( E3PetscSolverConfig::Solver_Name(config->solver()));
	adj_n_cores->set_value(config->n_cores());
}
