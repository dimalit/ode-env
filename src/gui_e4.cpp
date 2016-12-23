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

EXConfigWidget::EXConfigWidget(const OdeConfig* cfg){

	grid.set_column_spacing(4);
	grid.set_row_spacing(4);
	grid.set_margin_left(4);
	grid.set_margin_top(4);
	grid.set_margin_right(4);
	grid.set_margin_bottom(4);
	this->add(grid);

	grid.insert_row(0);

	button_apply.set_label("Apply");
	button_apply.set_hexpand(true);
	grid.attach(button_apply, 0, 0, 2, 1);

	if(!cfg){
		this->config = NULL;
		return;
	}
	else
		this->config = cfg->clone();

	const Message *msg = dynamic_cast<const Message*>(config);
	const Descriptor* d = msg->GetDescriptor();

	for(int i=0; i<d->field_count(); i++){
		const FieldDescriptor* fd = d->field(d->field_count()-i-1);
		const string& fname = fd->name();

		Gtk::Label *label = Gtk::manage(new Gtk::Label);
		label->set_text(fname);

		Gtk::Entry *entry = Gtk::manage(new Gtk::Entry);
		entry->set_hexpand(true);

		grid.insert_row(0);
		grid.attach(*label, 0, 0, 1, 1);
		grid.attach(*entry, 1, 0, 2, 1);

		entry_map[fname] = entry;
		entry->signal_changed().connect(sigc::mem_fun(*this, &EXConfigWidget::edit_anything_cb));
	}// for fields

	config_to_widget();

	button_apply.signal_clicked().connect(sigc::mem_fun(*this, &EXConfigWidget::on_apply_cb));
}

void EXConfigWidget::widget_to_config(){
	Message *msg = dynamic_cast<Message*>(config);
	const Descriptor* desc = msg->GetDescriptor();
	const Reflection* refl = msg->GetReflection();

	for(auto i=entry_map.begin(); i!=entry_map.end(); ++i){
		const string& var = i->first;
		Gtk::Entry* entry = i->second;

		string val = entry->get_text();

		const FieldDescriptor* fd = desc->FindFieldByName(var);
		FieldDescriptor::Type type = fd->type();

		switch(type){
			case FieldDescriptor::Type::TYPE_DOUBLE:
				refl->SetDouble(msg, fd, atof(val.c_str()));
				break;
			case FieldDescriptor::Type::TYPE_FLOAT:
				refl->SetFloat(msg, fd, (float)atof(val.c_str()));
				break;
			case FieldDescriptor::Type::TYPE_INT32:
				refl->SetInt32(msg, fd, atoi(val.c_str()));
				break;
			case FieldDescriptor::Type::TYPE_UINT32:
				refl->SetUInt32(msg, fd, atoi(val.c_str()));
				break;
			case FieldDescriptor::Type::TYPE_STRING:
				refl->SetString(msg, fd, val);
				break;
			default:
				assert(false);
		}

	}// for

	button_apply.set_sensitive(false);
}
void EXConfigWidget::config_to_widget(){

	Message *msg = dynamic_cast<Message*>(config);
	const Descriptor* desc = msg->GetDescriptor();
	const Reflection* refl = msg->GetReflection();

	for(auto i=entry_map.begin(); i!=entry_map.end(); ++i){
		const string& var = i->first;
		Gtk::Entry* entry = i->second;

		std::ostringstream res;

		const FieldDescriptor* fd = desc->FindFieldByName(var);
		FieldDescriptor::Type type = fd->type();

		switch(type){
			case FieldDescriptor::Type::TYPE_DOUBLE:
				res << refl->GetDouble(*msg, fd);
				break;
			case FieldDescriptor::Type::TYPE_FLOAT:
				res << refl->GetFloat(*msg, fd);
				break;
			case FieldDescriptor::Type::TYPE_INT32:
				res << refl->GetInt32(*msg, fd);
				break;
			case FieldDescriptor::Type::TYPE_UINT32:
				res << refl->GetUInt32(*msg, fd);
				break;
			case FieldDescriptor::Type::TYPE_STRING:
				res << refl->GetString(*msg, fd);
				break;
			default:
				assert(false);
		}

		entry->set_text(res.str());

	}// for

	button_apply.set_sensitive(false);
}

void EXConfigWidget::edit_anything_cb(){
	button_apply.set_sensitive(true);
}

void EXConfigWidget::on_apply_cb(){
	widget_to_config();
	m_signal_changed.emit();
}

void EXConfigWidget::loadConfig(const OdeConfig* cfg){
	const E4Config* ecfg = dynamic_cast<const E4Config*>(cfg);
		assert(ecfg);
	delete this->config;
	this->config = new E4Config(*ecfg);
	config_to_widget();

	m_signal_changed.emit();
}

const OdeConfig* EXConfigWidget::getConfig() {
	widget_to_config();
	return dynamic_cast<const OdeConfig*>(config);
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
	entry_a0->set_text("1.0");

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

EXPetscSolverConfigWidget::EXPetscSolverConfigWidget(const EXPetscSolverConfig* config){
	if(config)
		this->config = new EXPetscSolverConfig(*config);
	else
		this->config = new EXPetscSolverConfig();

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
const OdeSolverConfig* EXPetscSolverConfigWidget::getConfig(){
	widget_to_config();
	return config;
}
void EXPetscSolverConfigWidget::loadConfig(const OdeSolverConfig* config){
	const EXPetscSolverConfig* econfig = dynamic_cast<const EXPetscSolverConfig*>(config);
		assert(econfig);
	delete this->config;
	this->config = new EXPetscSolverConfig(*econfig);
	config_to_widget();
}

void EXPetscSolverConfigWidget::widget_to_config(){
	config->set_init_step(atof(entry_step->get_text().c_str()));
	config->set_atol(atof(entry_atol->get_text().c_str()));
	config->set_rtol(atof(entry_rtol->get_text().c_str()));
	config->set_n_cores(adj_n_cores->get_value());
}
void EXPetscSolverConfigWidget::config_to_widget(){
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
