/*
 * gui_e2.cpp
 *
 *  Created on: Jul 14, 2014
 *      Author: dimalit
 */

#include "gui_e2.h"

#include "rpc.h"

#include <gtkmm/builder.h>

#define UI_FILE_CONF "e2_config.glade"
#define UI_FILE_STATE "e2_state.glade"
#define UI_FILE_PETSC_SOLVER "e2_petsc_solver.glade"

E2ConfigWidget::E2ConfigWidget(const E2Config* cfg){
	if(cfg)
		this->config = new E2Config(*cfg);
	else
		this->config = new E2Config();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_CONF);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_n", entry_n);
	b->get_widget("entry_a", entry_a);

	b->get_widget("entry_delta", entry_delta);
	b->get_widget("entry_f", entry_f);

	b->get_widget("button_apply", button_apply);

	this->add(*root);

	config_to_widget();

	entry_n->signal_changed().connect(sigc::mem_fun(*this, &E2ConfigWidget::edit_anything_cb));
	entry_a->signal_changed().connect(sigc::mem_fun(*this, &E2ConfigWidget::edit_anything_cb));
	entry_delta->signal_changed().connect(sigc::mem_fun(*this, &E2ConfigWidget::edit_anything_cb));
	entry_f->signal_changed().connect(sigc::mem_fun(*this, &E2ConfigWidget::edit_anything_cb));

	button_apply->signal_clicked().connect(sigc::mem_fun(*this, &E2ConfigWidget::on_apply_cb));
}

void E2ConfigWidget::widget_to_config(){
	config->set_n(atoi(entry_n->get_text().c_str()));
	config->set_a(atoi(entry_a->get_text().c_str()));
	config->set_delta(atoi(entry_delta->get_text().c_str()));
	config->set_f(atoi(entry_f->get_text().c_str()));

	button_apply->set_sensitive(false);
}
void E2ConfigWidget::config_to_widget(){
	std::ostringstream buf;
	buf << config->n();
	entry_n->set_text(buf.str());

	buf.str("");
	buf << config->a();
	entry_a->set_text(buf.str());

	buf.str("");
	buf << config->delta();
	entry_delta->set_text(buf.str());

	buf.str("");
	buf << config->f();
	entry_f->set_text(buf.str());

	button_apply->set_sensitive(false);
}

void E2ConfigWidget::edit_anything_cb(){
	button_apply->set_sensitive(true);
}

void E2ConfigWidget::on_apply_cb(){
	widget_to_config();
	m_signal_changed.emit();
}

void E2ConfigWidget::loadConfig(const OdeConfig* cfg){
	const E2Config* ecfg = dynamic_cast<const E2Config*>(cfg);
		assert(ecfg);
	delete this->config;
	this->config = new E2Config(*ecfg);
	config_to_widget();

	m_signal_changed.emit();
}

const OdeConfig* E2ConfigWidget::getConfig() {
	widget_to_config();
	return config;
}

E2StateWidget::E2StateWidget(const E2Config* _config, const E2State* _state){
	to_gnuplot = NULL;

	if(_config)
		this->config = new E2Config(*_config);
	else
		this->config = new E2Config();

	if(_state)
		this->state = new E2State(*_state);
	else
		// TODO: may be state should remember its config?!
		this->state = new E2State(this->config);

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_STATE);

	Gtk::Widget* root;
	b->get_widget("root", root);

	Gtk::Container* place_for_socket;
	b->get_widget("place_for_socket", place_for_socket);
	place_for_socket->add(this->x11_socket);

	b->get_widget("entry_left", entry_left);
	b->get_widget("entry_right", entry_right);

	b->get_widget("radio_rand", radio_rand);
	b->get_widget("radio_linear", radio_linear);

	b->get_widget("label_i_left", label_i_left);
	b->get_widget("label_phi_left", label_phi_left);
	b->get_widget("label_e", label_e);

	b->get_widget("button_apply", button_apply);

	this->add(*root);

	state_to_widget();
	if(!this->state->simulated())
		generateState();

	// signals
	this->signal_realize().connect(sigc::mem_fun(this, &E2StateWidget::on_realize_cb));

	entry_left->signal_changed().connect(sigc::mem_fun(*this, &E2StateWidget::edit_anything_cb));
	entry_right->signal_changed().connect(sigc::mem_fun(*this, &E2StateWidget::edit_anything_cb));
	radio_rand->signal_toggled().connect(sigc::mem_fun(*this, &E2StateWidget::edit_anything_cb));

	button_apply->signal_clicked().connect(sigc::mem_fun(*this, &E2StateWidget::on_apply_cb));
}

E2StateWidget::~E2StateWidget(){
	if(to_gnuplot)
		fclose(to_gnuplot);
}

void E2StateWidget::widget_to_state(){
	if(!state->simulated()){
		generateState();
	}
}
void E2StateWidget::state_to_widget(){

	if(state->simulated()){
		update_chart();
	}
}

void E2StateWidget::edit_anything_cb(){
	button_apply->set_sensitive(true);
}
void E2StateWidget::on_apply_cb(){
	generateState();
	m_signal_changed.emit();
}


void E2StateWidget::loadConfig(const OdeConfig* cfg){
	const E2Config* ecfg = dynamic_cast<const E2Config*>(cfg);
		assert(ecfg);
	delete this->config;
	this->config = new E2Config(*ecfg);

	delete this->state;
	this->state = new E2State(ecfg);

	state_to_widget();
	widget_to_state();

	m_signal_changed.emit();
}
const OdeConfig* E2StateWidget::getConfig(){
	return config;
}

void E2StateWidget::loadState(const OdeState* state){
	const E2State* estate = dynamic_cast<const E2State*>(state);
		assert(estate);
	delete this->state;
	this->state = new E2State(*estate);

	state_to_widget();
	widget_to_state();

	m_signal_changed.emit();
}
const OdeState* E2StateWidget::getState(){
	widget_to_state();
	return state;
}

void E2StateWidget::generateState(){
	double left = atof(entry_left->get_text().c_str());
	double right = atof(entry_right->get_text().c_str());

	bool use_rand = this->radio_rand->get_active();

	int n = config->n();
	for(int i=0; i<n; i++){
		double ksi;
		if(use_rand)
			ksi = rand() / (double)RAND_MAX * (right-left) + left;
		else
			ksi = i / (double)n * (right-left) + left;

		pb::E2State::Particles p;
		p.set_ksi(ksi); p.set_v(0.0);

		state->mutable_particles(i)->CopyFrom(p);
	}

	state->set_simulated(false);

	update_chart();
}


void E2StateWidget::update_chart(){
	// TODO: Написать как направлять команды gnuplot'у и вставлять его вывод в окно!

	if(!this->is_realized())
		return;

	if(to_gnuplot == NULL){
		int rf, wf;
		rpc_call("gnuplot", &rf, &wf);
	//	close(rf);

		to_gnuplot = fdopen(wf, "wb");
		if(!to_gnuplot){
			perror(NULL);
			assert(false);
		}
	}

	unsigned id = x11_socket.get_id();
		assert(id != 0);

	fprintf(to_gnuplot, "set terminal x11 window \"%x\"\n", id);
	fprintf(to_gnuplot, "set yrange [-1:2]\n");
	fprintf(to_gnuplot, "plot '-' using 1:2 with points\n");
//	fprintf(wff, "plot [-3.14:3.14] sin(x)\n");

	// print data
	for(int i=0; i<state->particles_size(); i++){
		fprintf(to_gnuplot, "%lf %lf\n", state->particles(i).ksi(), state->particles(i).v());
	}
	fprintf(to_gnuplot, "e\n");
	fflush(to_gnuplot);

}

void E2StateWidget::on_realize_cb(){
	update_chart();
}

E2PetscSolverConfigWidget::E2PetscSolverConfigWidget(const E2PetscSolverConfig* config){
	if(config)
		this->config = new E2PetscSolverConfig(*config);
	else
		this->config = new E2PetscSolverConfig();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_PETSC_SOLVER);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_tol", entry_tol);
	b->get_widget("entry_step", entry_step);

	this->add(*root);

	config_to_widget();
}
const OdeSolverConfig* E2PetscSolverConfigWidget::getConfig(){
	widget_to_config();
	return config;
}
void E2PetscSolverConfigWidget::loadConfig(const OdeSolverConfig* config){
	const E2PetscSolverConfig* econfig = dynamic_cast<const E2PetscSolverConfig*>(config);
		assert(econfig);
	delete this->config;
	this->config = new E2PetscSolverConfig(*econfig);
	config_to_widget();
}

void E2PetscSolverConfigWidget::widget_to_config(){
	config->set_init_step(atof(entry_step->get_text().c_str()));
	config->set_tolerance(atof(entry_tol->get_text().c_str()));
}
void E2PetscSolverConfigWidget::config_to_widget(){
	std::ostringstream buf;
	buf << config->init_step();
	entry_step->set_text(buf.str());

	buf.str("");
	buf << config->tolerance();
	entry_tol->set_text(buf.str());
}
