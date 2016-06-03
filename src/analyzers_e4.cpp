/*
 * e3_analyzers.cpp
 *
 *  Created on: Aug 22, 2014
 *      Author: dimalit
 */

#include "analyzers_e4.h"

#include <cassert>

#define UI_FILE "e4_conservation.glade"

// TODO: make Gnuplotting" functionality apart and use it to draw energy chars!
E4ConservationAnalyzer::E4ConservationAnalyzer(const E4Config* config){
	states_count = 0;

	this->config = new E4Config(*config);

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_int1", entry_int1);
	b->get_widget("entry_int2", entry_int2);
	b->get_widget("entry_e", entry_e);
	b->get_widget("entry_phi", entry_phi);

	b->get_widget("treeview1", treeview1);
	liststore1 = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(b->get_object("liststore1"));

	this->add(*root);
}

void E4ConservationAnalyzer::loadConfig(const OdeConfig* config){
	const E4Config* econfig = dynamic_cast<const E4Config*>(config);
		assert(econfig);

	this->config = new E4Config(*econfig);
}

void E4ConservationAnalyzer::reset(){
	states_count = 0;
}
void E4ConservationAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const E4State* estate = dynamic_cast<const E4State*>(state);
		assert(estate);

	liststore1->clear();

	double sum_a_2 = 0;
	for(int i=0; i<estate->particles_size(); i++){
		E4State::Particles p = estate->particles(i);

		Gtk::ListStore::iterator it = liststore1->append();
		it->set_value(0, p.a());
		it->set_value(1, p.psi());
		it->set_value(2, p.z());
		it->set_value(3, p.delta());

		sum_a_2 += p.a()*p.a();
	}

	std::ostringstream buf;
	buf.setf( std::ios::fixed, std:: ios::floatfield );
	buf.precision(10);

	buf.str("");
	buf << estate->e()*estate->e()+1.0/config->n()*sum_a_2;
	entry_int1->set_text(buf.str());

	buf.str("");
	buf << estate->e();
	entry_e->set_text(buf.str());

	buf.str("");
	buf << estate->phi();
	entry_phi->set_text(buf.str());

	++states_count;
	last_update = ::time(NULL);
}

int E4ConservationAnalyzer::getStatesCount(){
	return states_count;
}

E4ConservationAnalyzer::~E4ConservationAnalyzer(){
	delete config;
}

