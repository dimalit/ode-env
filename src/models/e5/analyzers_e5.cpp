/*
 * e3_analyzers.cpp
 *
 *  Created on: Aug 22, 2014
 *      Author: dimalit
 */

#include <models/e5/analyzers_e5.h>
#include <cassert>

#define UI_FILE "src/models/e5/e5_conservation.glade"

// TODO: maove it from E42mc to separate unit
double arg(double x, double y);
double mod(double x, double y);
double mod2(double x, double y);
void rotate(double& x, double& y, double alpha);

// TODO: make Gnuplotting" functionality apart and use it to draw energy chars!
E5ConservationAnalyzer::E5ConservationAnalyzer(const E5Config* config){
	states_count = 0;

	this->config = new E5Config(*config);

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_int1", entry_int1);
	b->get_widget("entry_int2", entry_int2);


	b->get_widget("entry_aver_x", entry_aver_x);
	b->get_widget("entry_aver_y", entry_aver_y);

	b->get_widget("entry_cm_r", entry_cm_r);

	b->get_widget("treeview1", treeview1);
	liststore1 = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(b->get_object("liststore1"));

	this->add(*root);
}

void E5ConservationAnalyzer::loadConfig(const OdeConfig* config){
	const E5Config* econfig = dynamic_cast<const E5Config*>(config);
		assert(econfig);

	this->config = new E5Config(*econfig);
}

void E5ConservationAnalyzer::reset(){
	states_count = 0;
}
void E5ConservationAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const E5State* estate = dynamic_cast<const E5State*>(state);
		assert(estate);

	liststore1->clear();

	double sum_a_2 = 0;
	double sum_x = 0, sum_y = 0;

	for(int i=0; i<estate->particles_size(); i++){
		E5State::Particles p = estate->particles(i);
		E5State::Fields f = estate->fields(i);

		Gtk::ListStore::iterator it = liststore1->append();
		it->set_value(0, mod(p.x(), p.y()));
		it->set_value(1, arg(p.x(), p.y()));
		it->set_value(2, p.z());
		it->set_value(3, mod(f.x(), f.y()));
		it->set_value(4, arg(f.x(), f.y()));

		sum_a_2 += mod2(p.x(), p.y());

		sum_x += p.x();
		sum_y += p.y();
	}

	std::ostringstream buf;
	buf.setf( std::ios::fixed, std:: ios::floatfield );
	buf.precision(10);

//	buf.str("");
//	buf << mod2(estate->x_p(), estate->y_p()) + mod2(estate->x_m(), estate->y_m()) + 1.0/config->n()*sum_a_2+estate->eout();
//	entry_int1->set_text(buf.str());

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

int E5ConservationAnalyzer::getStatesCount(){
	return states_count;
}

E5ConservationAnalyzer::~E5ConservationAnalyzer(){
	delete config;
}

E5ChartAnalyzer::E5ChartAnalyzer(const OdeConfig* config):EXChartAnalyzer(config) {
}

E5ChartAnalyzer::~E5ChartAnalyzer() {

}

void E5ChartAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(state);
		assert(msg);
	const google::protobuf::Message* d_msg = dynamic_cast<const google::protobuf::Message*>(d_state);
//		assert(d_msg);

	for(int i=0; i<charts.size(); i++)
			charts[i]->processMessage(msg, d_msg, time);
}

void E5ChartAnalyzer::addChart(MessageChart* chart){
	EXChartAnalyzer::addChart(chart);
}

void E5ChartAnalyzer::addSpecial(MessageChart* chart){
	charts.push_back(chart);
	vbox.pack_end(*chart, false, false, 1);
	this->show_all();
}

