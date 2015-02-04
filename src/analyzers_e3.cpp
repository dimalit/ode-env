/*
 * e3_analyzers.cpp
 *
 *  Created on: Aug 22, 2014
 *      Author: dimalit
 */

#include "analyzers_e3.h"

#include <cassert>

#define UI_FILE "e3_conservation.glade"

E3ConservationAnalyzer::E3ConservationAnalyzer(const E3Config* config){
	states_count = 0;

	this->config = new E3Config(*config);

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_i1", entry_i1);
	b->get_widget("entry_i2", entry_i2);
	b->get_widget("entry_i3", entry_i3);

	b->get_widget("treeview1", treeview1);
	liststore1 = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(b->get_object("liststore1"));

	this->add(*root);
}

void E3ConservationAnalyzer::loadConfig(const OdeConfig* config){
	const E3Config* econfig = dynamic_cast<const E3Config*>(config);
		assert(econfig);

	this->config = new E3Config(*econfig);
}

void E3ConservationAnalyzer::reset(){
	states_count = 0;
}
void E3ConservationAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const E3State* estate = dynamic_cast<const E3State*>(state);
		assert(estate);

	liststore1->clear();

	double i1 = 0;
	double sum_a_2 = 0;
	double sum_eta = 0;
	for(int i=0; i<estate->particles_size(); i++){
		E3State::Particles p = estate->particles(i);

		Gtk::ListStore::iterator it = liststore1->append();
		it->set_value(0, p.eta());
		it->set_value(1, p.a());
		it->set_value(2, p.ksi());
		double integral = config->r_e()*p.a()*p.a() - 2*config->n()*p.eta();
		it->set_value(3, integral);

		i1 += integral;
		sum_a_2 += p.a()*p.a();
		sum_eta += p.eta();
	}

	std::ostringstream buf;
	buf << i1;
	entry_i1->set_text(buf.str());

	buf.str("");
	buf << estate->e()*estate->e()+2.0/config->r_e()/config->m()*sum_eta;
	entry_i2->set_text(buf.str());

	buf.str("");
	buf << estate->e()*estate->e()+1.0/config->n()/config->m()*sum_a_2;
	entry_i3->set_text(buf.str());

	++states_count;
}

int E3ConservationAnalyzer::getStatesCount(){
	return states_count;
}

E3ConservationAnalyzer::~E3ConservationAnalyzer(){
	delete config;
}

