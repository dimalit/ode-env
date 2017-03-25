/*
 * e3_analyzers.cpp
 *
 *  Created on: Aug 22, 2014
 *      Author: dimalit
 */

#include <models/e42mc/analyzers_e42mc.h>
#include <cassert>

#define UI_FILE "src/models/e42mc/e42mc_conservation.glade"

double arg(double x, double y){
	return atan2(y,x);
}

double mod(double x, double y){
	return sqrt(x*x+y*y);
}

double mod2(double x, double y){
	return x*x+y*y;
}

// TODO: make Gnuplotting" functionality apart and use it to draw energy chars!
E42mcConservationAnalyzer::E42mcConservationAnalyzer(const E42mcConfig* config){
	states_count = 0;

	this->config = new E42mcConfig(*config);

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_int1", entry_int1);
	b->get_widget("entry_int2", entry_int2);
	b->get_widget("entry_e_p", entry_e_p);
	b->get_widget("entry_phi_p", entry_phi_p);
	b->get_widget("entry_e_m", entry_e_m);
	b->get_widget("entry_phi_m", entry_phi_m);

	b->get_widget("entry_aver_x", entry_aver_x);
	b->get_widget("entry_aver_y", entry_aver_y);
	b->get_widget("entry_cm_r", entry_cm_r);

	b->get_widget("treeview1", treeview1);
	liststore1 = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(b->get_object("liststore1"));

	this->add(*root);
}

void E42mcConservationAnalyzer::loadConfig(const OdeConfig* config){
	const E42mcConfig* econfig = dynamic_cast<const E42mcConfig*>(config);
		assert(econfig);

	this->config = new E42mcConfig(*econfig);
}

void E42mcConservationAnalyzer::reset(){
	states_count = 0;
}
void E42mcConservationAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const E42mcState* estate = dynamic_cast<const E42mcState*>(state);
		assert(estate);

	liststore1->clear();

	double sum_a_2 = 0;
	double sum_x = 0, sum_y = 0;

	for(int i=0; i<estate->particles_size(); i++){
		E42mcState::Particles p = estate->particles(i);

		Gtk::ListStore::iterator it = liststore1->append();
		it->set_value(0, mod(p.x(), p.y()));
		it->set_value(1, arg(p.x(), p.y()));
		it->set_value(2, p.z());
		it->set_value(3, mod(p.xn(), p.yn()));
		it->set_value(4, arg(p.xn(), p.yn()));

		sum_a_2 += mod2(p.x(), p.y());
	}

	std::ostringstream buf;
	buf.setf( std::ios::fixed, std:: ios::floatfield );
	buf.precision(10);

	buf.str("");
	buf << mod2(estate->x_p(), estate->y_p()) + mod2(estate->x_m(), estate->y_m()) + 1.0/config->n()*sum_a_2;
	entry_int1->set_text(buf.str());

	buf.str("");
	buf << mod(estate->x_p(), estate->y_p());
	entry_e_p->set_text(buf.str());

	buf.str("");
	buf << arg(estate->x_p(), estate->y_p());
	entry_phi_p->set_text(buf.str());

	buf.str("");
	buf << mod(estate->x_m(), estate->y_m());
	entry_e_m->set_text(buf.str());

	buf.str("");
	buf << arg(estate->x_m(), estate->y_m());
	entry_phi_m->set_text(buf.str());

	buf.str("");
	buf << sum_x/estate->particles_size();
	entry_aver_x->set_text(buf.str());

	buf.str("");
	buf << sum_y/estate->particles_size();
	entry_aver_y->set_text(buf.str());

	buf.str("");
	buf << sqrt(sum_x*sum_x + sum_y*sum_y)/estate->particles_size();
	entry_cm_r->set_text(buf.str());

	++states_count;
	last_update = ::time(NULL);
}

int E42mcConservationAnalyzer::getStatesCount(){
	return states_count;
}

E42mcConservationAnalyzer::~E42mcConservationAnalyzer(){
	delete config;
}

E42mcChartAnalyzer::E42mcChartAnalyzer(const OdeConfig* config):EXChartAnalyzer(config) {
}

E42mcChartAnalyzer::~E42mcChartAnalyzer() {

}

void E42mcChartAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(state);
		assert(msg);
	const google::protobuf::Message* d_msg = dynamic_cast<const google::protobuf::Message*>(d_state);
		assert(d_msg);

	for(int i=0; i<charts.size(); i++)
			charts[i]->processMessage(msg, d_msg, time);
}

void E42mcChartAnalyzer::addChart(MessageChart* chart){
	EXChartAnalyzer::addChart(chart);
}

void E42mcChartAnalyzer::addSpecial(MessageChart* chart){
	charts.push_back(chart);
	vbox.pack_end(*chart, false, false, 1);
	this->show_all();
}

