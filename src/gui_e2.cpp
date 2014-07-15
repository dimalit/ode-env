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

	this->add(*root);

	config_to_widget();
}

void E2ConfigWidget::widget_to_config(){
	config->set_n(atoi(entry_n->get_text().c_str()));
	config->set_a(atoi(entry_a->get_text().c_str()));
	config->set_delta(atoi(entry_delta->get_text().c_str()));
	config->set_f(atoi(entry_f->get_text().c_str()));
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
}

void E2ConfigWidget::loadConfig(const OdeConfig* cfg){
	const E2Config* ecfg = dynamic_cast<const E2Config*>(cfg);
		assert(ecfg);
	delete this->config;
	this->config = new E2Config(*ecfg);
	config_to_widget();
}

const OdeConfig* E2ConfigWidget::getConfig() {
	widget_to_config();
	return config;
}

E2StateWidget::E2StateWidget(const E2Config* _config, const E2State* _state){
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

	this->add(*root);

	// signals
	Gtk::Button* apply;
	b->get_widget("button_apply", apply);
	apply->signal_clicked().connect(sigc::mem_fun(*this, &E2StateWidget::apply_cb));

	state_to_widget();
}
void E2StateWidget::loadState(const OdeState* state){
	const E2State* estate = dynamic_cast<const E2State*>(state);
		assert(estate);
	delete this->state;
	this->state = new E2State(*estate);

	state_to_widget();
}
const OdeState* E2StateWidget::getState(){
	widget_to_state();
	return state;
}

void E2StateWidget::loadConfig(const OdeConfig* cfg){
	const E2Config* ecfg = dynamic_cast<const E2Config*>(cfg);
		assert(ecfg);
	delete this->config;
	this->config = new E2Config(*ecfg);

	delete this->state;
	this->state = new E2State(ecfg);
}
const OdeConfig* E2StateWidget::getConfig(){
	return config;
}

void E2StateWidget::widget_to_state(){
	bool use_rand = this->radio_rand->get_active();

	double left = atof(entry_left->get_text().c_str());
	double right = atof(entry_right->get_text().c_str());

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

	update_chart();
}
void E2StateWidget::state_to_widget(){

}

void E2StateWidget::apply_cb(){
	update_chart();
}

void E2StateWidget::update_chart(){
	std::cout << "update_chart()" << std::endl;

	// TODO: Написать как направлять команды gnuplot'у и вставлять его вывод в окно!

	int rf, wf;
	rpc_call("gnuplot", &rf, &wf);
//	close(rf);

	FILE* wff = fdopen(wf, "wb");
	if(!wff){
		perror(NULL);
		assert(false);
	}
	fprintf(wff, "set terminal x11 window \"%x\"\n", x11_socket.get_id());
	fprintf(wff, "plot [-3.14:3.14] sin(x)\n");
	fflush(wff);
//	fclose(wff);
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
