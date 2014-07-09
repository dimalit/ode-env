/*
 * gui_e1.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: dimalit
 */

#include "gui_e1.h"

#include <gtkmm/builder.h>
#include <gtkmm/frame.h>
#include <gtkmm/alignment.h>

#include <string>

#define UI_FILE_CONF "e1_config.glade"
#define UI_FILE_STATE "e1_state.glade"
#define UI_FILE_PETSC_SOLVER "e1_petsc_solver.glade"

E1InstanceWidgetFactory E1InstanceWidgetFactory::instance;
E1PetscSolverConfigWidgetFactory E1PetscSolverConfigWidgetFactory::instance;

E1ConfigWidget::E1ConfigWidget(const E1Config* cfg){
	if(cfg)
		this->config = new E1Config(*cfg);
	else
		this->config = new E1Config();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_CONF);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_m", entry_m);
	b->get_widget("entry_ksi", entry_ksi);

	b->get_widget("check_rand", check_rand);
	b->get_widget("check_linear", check_linear);

	this->add(*root);

	// callbacks
	check_rand->signal_toggled() .connect(sigc::mem_fun(*this, &E1ConfigWidget::check_rand_toggled_cb));
	check_linear->signal_toggled().connect(sigc::mem_fun(*this, &E1ConfigWidget::check_linear_toggled_cb));

	config_to_widget();
}

void E1ConfigWidget::widget_to_config(){
	config->g_m = atoi(entry_m->get_text().c_str());
	config->ksi = atof(entry_ksi->get_text().c_str());
	config->random_ksi = check_rand->get_active();
	config->linear_ksi = check_linear->get_active();
}
void E1ConfigWidget::config_to_widget(){
	std::ostringstream buf;
	buf << config->g_m;
	entry_m->set_text(buf.str());

	buf.str("");
	buf << config->ksi;
	entry_ksi->set_text(buf.str());

	check_rand->set_active(config->random_ksi);
	check_linear->set_active(config->linear_ksi);
}

void E1ConfigWidget::check_rand_toggled_cb(){
	gboolean me = check_rand->get_active();
	gboolean other = check_linear->get_active();
	if(me && other)
		check_linear->set_active(false);
	entry_ksi->set_sensitive(!me && !other);

}
void E1ConfigWidget::check_linear_toggled_cb(){
	gboolean me = check_linear->get_active();
	gboolean other = check_rand->get_active();
	if(me && other)
		check_rand->set_active(false);
	entry_ksi->set_sensitive(!me && !other);
}

void E1ConfigWidget::loadConfig(const OdeConfig* cfg){
	const E1Config* ecfg = dynamic_cast<const E1Config*>(cfg);
		assert(ecfg);
	delete this->config;
	this->config = new E1Config(*ecfg);
	config_to_widget();
}

const OdeConfig* E1ConfigWidget::getConfig() {
	widget_to_config();
	return config;
}

E1StateWidget::E1StateWidget(const E1Config* _config, const E1State* _state){
	if(_config)
		this->config = new E1Config(*_config);
	else
		this->config = new E1Config();

	if(_state)
		this->state = new E1State(*_state);
	else
		// TODO: may be state should remember its config?!
		this->state = new E1State(this->config);

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_STATE);

	Gtk::Widget* root;
	b->get_widget("root", root);

	adj_E = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(b->get_object("adj_E"));
	adj_phi = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(b->get_object("adj_phi"));
	adj_b = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(b->get_object("adj_b"));

	b->get_widget("label_phi", label_phi);

	this->add(*root);

	// chart:
	series_E_xs[0] = 0.0;
	series_E_xs[1] = 1.0;
	series_b_xs = new double[config->g_m];
	series_b_ys = new double[config->g_m];

	series_phi_data_x = NULL;
	series_phi_data_y = NULL;

	ggw = go_graph_widget_new(NULL);
	gtk_widget_set_size_request(ggw, 500, 300);
	draw(go_graph_widget_get_chart(GO_GRAPH_WIDGET(ggw)));
	Gtk::Alignment* place_for_chart;
	b->get_widget("place_for_chart", place_for_chart);

	// TODO: add c to C++ instead of reverse
	gtk_container_add(GTK_CONTAINER(place_for_chart->gobj()), ggw);

	// callbacks
	adj_E->signal_value_changed().connect(sigc::mem_fun(*this, &E1StateWidget::adj_E_value_changed_cb));
	adj_b->signal_value_changed().connect(sigc::mem_fun(*this, &E1StateWidget::adj_b_value_changed_cb));
	adj_phi->signal_value_changed().connect(sigc::mem_fun(*this, &E1StateWidget::adj_phi_value_changed_cb));

	state_to_widget();
}
void E1StateWidget::loadState(const OdeState* state){
	const E1State* estate = dynamic_cast<const E1State*>(state);
		assert(estate);
	delete this->state;
	this->state = new E1State(*estate);

	state_to_widget();
}
const OdeState* E1StateWidget::getState(){
	widget_to_state();
	return state;
}

void E1StateWidget::loadConfig(const OdeConfig* cfg){
	const E1Config* ecfg = dynamic_cast<const E1Config*>(cfg);
		assert(ecfg);
	delete this->config;
	this->config = new E1Config(*ecfg);

	delete this->state;
	this->state = new E1State(ecfg);
}
const OdeConfig* E1StateWidget::getConfig(){
	return config;
}

void E1StateWidget::widget_to_state(){
	state->setE(adj_E->get_value());
	state->setB(adj_b->get_value());
	state->setPhi(adj_phi->get_value());
}
void E1StateWidget::state_to_widget(){
	adj_E->set_value(state->getE());
	adj_b->set_value(state->getB());
	adj_phi->set_value(state->getPhi());

	// draw particles
	for(int i=0; i<20; i++){
		std::cout << state->getBArray()[i] << " ";
	}
	std::cout << std::endl;

	std::copy(state->getBArray().begin(), state->getBArray().end(), this->series_b_ys);
	std::copy(state->getKsiArray().begin(), state->getKsiArray().end(), this->series_b_xs);
	go_data_emit_changed(series_b_data);
}

void E1StateWidget::draw(GogChart* chart){
	GogPlot *plot;
	GError *error;

	plot = (GogPlot *) gog_plot_new_by_name ("GogXYPlot");

	series_E = gog_plot_new_series (plot);
	gog_series_set_name(series_E, GO_DATA_SCALAR(go_data_scalar_str_new("E", 0)), &error);
	series_E_data = go_data_vector_val_new (series_E_xs, 2, NULL);
	gog_series_set_dim (series_E, 0, series_E_data, &error);
	series_E_data = go_data_vector_val_new (series_E_ys, 2, NULL);
	gog_series_set_dim (series_E, 1, series_E_data, &error);
	adj_E_value_changed_cb();

	series_b = gog_plot_new_series (plot);
	gog_series_set_name(series_b, GO_DATA_SCALAR(go_data_scalar_str_new("b", 0)), &error);
	series_b_data = go_data_vector_val_new (series_b_xs, config->g_m, NULL);
	gog_series_set_dim (series_b, 0, series_b_data, &error);
	series_b_data = go_data_vector_val_new (series_b_ys, config->g_m, NULL);
	gog_series_set_dim (series_b, 1, series_b_data, &error);
	adj_b_value_changed_cb();

	series_phi = gog_plot_new_series (plot);
	gog_series_set_name(series_phi, GO_DATA_SCALAR(go_data_scalar_str_new("phi", 0)), &error);
	for(int i=0; i<21; i++){
		double x = i / 20.0;
		series_phi_xs[i] = x;
	}
	series_phi_data_x = go_data_vector_val_new (series_phi_xs, 21, NULL);
	gog_series_set_dim (series_phi, 0, series_phi_data_x, &error);
	series_phi_data_y = go_data_vector_val_new (series_phi_ys, 21, NULL);
	gog_series_set_dim (series_phi, 1, series_phi_data_y, &error);
	adj_phi_value_changed_cb();

	gog_object_add_by_name (GOG_OBJECT (chart), "Plot", GOG_OBJECT (plot));
	gog_object_add_by_name (GOG_OBJECT (chart), "Legend", NULL);
}

void E1StateWidget::adj_E_value_changed_cb (){
	double val = adj_E->get_value();
	series_E_ys[0] = val;
	series_E_ys[1] = val;

	go_data_emit_changed(series_E_data);
}
void E1StateWidget::adj_b_value_changed_cb (){
	double val = adj_b->get_value();
	for(int i=0; i<config->g_m; i++){
		series_b_ys[i] = val;
	}

	go_data_emit_changed(series_b_data);
	// change also sine profile
	adj_phi_value_changed_cb();
}
void E1StateWidget::adj_phi_value_changed_cb (){
	// change label
	double val = adj_phi->get_value();
	char buf[10]; sprintf(buf, "phi %.2f", val);
	label_phi->set_text(buf);

	// change plot
	double hi = adj_b->get_value();
	for(int i=0; i<21; i++){
		double x = i / 20.0;
		series_phi_ys[i] = hi - sin(x*2*M_PI + val)/10.0;
	}
	// first time we are called before construction from b callback
	if(series_phi_data_y)
		go_data_emit_changed(series_phi_data_y);
}

E1PetscSolverConfigWidget::E1PetscSolverConfigWidget(const E1PetscSolverConfig* config){
	if(config)
		this->config = new E1PetscSolverConfig(*config);
	else
		this->config = new E1PetscSolverConfig();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_PETSC_SOLVER);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_tol", entry_tol);
	b->get_widget("entry_step", entry_step);

	this->add(*root);
}
const OdeSolverConfig* E1PetscSolverConfigWidget::getConfig(){
	widget_to_config();
	return config;
}
void E1PetscSolverConfigWidget::loadConfig(const OdeSolverConfig* config){
	const E1PetscSolverConfig* econfig = dynamic_cast<const E1PetscSolverConfig*>(config);
		assert(econfig);
	delete this->config;
	this->config = new E1PetscSolverConfig(*econfig);
	config_to_widget();
}

void E1PetscSolverConfigWidget::widget_to_config(){
	config->g_step = atof(entry_step->get_text().c_str());
	config->g_tol = atof(entry_tol->get_text().c_str());
}
void E1PetscSolverConfigWidget::config_to_widget(){
	std::ostringstream buf;
	buf << config->g_step;
	entry_step->set_text(buf.str());

	buf.str("");
	buf << config->g_tol;
	entry_tol->set_text(buf.str());
}

/////////////////////////////////////////////////////////////////////

std::string E1InstanceWidgetFactory::getDisplayName() const {
	return "GTK+ widgets for E1";
}
OdeConfigWidget* E1InstanceWidgetFactory::createConfigWidget(const OdeConfig* cfg) const {
	const E1Config* ecfg = dynamic_cast<const E1Config*>(cfg);
		assert(ecfg || !cfg);		// it exists or other inexists
	return new E1ConfigWidget(ecfg);
}
OdeStateWidget* E1InstanceWidgetFactory::createStateWidget(const OdeConfig* cfg, const OdeState* state) const {
	const E1Config* ecfg = dynamic_cast<const E1Config*>(cfg);
		assert(ecfg);
	const E1State* estate = dynamic_cast<const E1State*>(state);
		assert(estate || !state);		// it exists or other inexists
	return new E1StateWidget(ecfg, estate);
}

std::string E1PetscSolverConfigWidgetFactory::getDisplayName() const {
	return "PETSc solver for E1 config widget";
}

OdeSolverConfigWidget* E1PetscSolverConfigWidgetFactory::createConfigWidget(const OdeSolverConfig* cfg) const {
	const E1PetscSolverConfig* ecfg = dynamic_cast<const E1PetscSolverConfig*>(cfg);
		assert(ecfg || !cfg);
	return new E1PetscSolverConfigWidget(ecfg);
}
