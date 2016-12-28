/*
 * e3_analyzers.cpp
 *
 *  Created on: Aug 22, 2014
 *      Author: dimalit
 */

#include "analyzers_e3.h"

#include <cassert>

#define UI_FILE "src/models/e3/e3_conservation.glade"

// TODO: make Gnuplotting" functionality apart and use it to draw energy chars!
E3ConservationAnalyzer::E3ConservationAnalyzer(const E3Config* config){
	states_count = 0;

	this->config = new E3Config(*config);

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_i1", entry_i1);
	b->get_widget("entry_i2", entry_i2);
	b->get_widget("entry_i3", entry_i3);
	b->get_widget("entry_e", entry_e);
	b->get_widget("entry_phi", entry_phi);

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
		//double delta = sin(2*M_PI*p.ksi() + estate->phi());
		it->set_value(3, integral);

		i1 += integral;
		sum_a_2 += p.a()*p.a();
		sum_eta += p.eta();
	}

	std::ostringstream buf;
	buf.setf( std::ios::fixed, std:: ios::floatfield );
	buf.precision(10);

	buf << i1/estate->particles_size();
	entry_i1->set_text(buf.str());

	buf.str("");		// 4.0 for alternate formula
	buf << estate->e()*estate->e()+2.0/config->r_e()/config->m()*sum_eta;
	entry_i2->set_text(buf.str());

	buf.str("");		// 2.0 for alternate formula
	buf << estate->e()*estate->e()+1.0/config->n()/config->m()*sum_a_2;
	entry_i3->set_text(buf.str());

	buf.str("");
	buf << estate->e();
	entry_e->set_text(buf.str());

	buf.str("");
	buf << estate->phi();
	entry_phi->set_text(buf.str());

	++states_count;
	last_update = ::time(NULL);
}

int E3ConservationAnalyzer::getStatesCount(){
	return states_count;
}

E3ConservationAnalyzer::~E3ConservationAnalyzer(){
	delete config;
}

E3ChartAnalyzer::E3ChartAnalyzer(const OdeConfig* config) {
	this->config = config->clone();

	states_count = 0;

	btn_add.set_label("Add chart");
	vbox.pack_start(btn_add);
	btn_add.signal_clicked().connect(sigc::mem_fun(*this, &E3ChartAnalyzer::on_add_clicked));

	btn_reset.set_label("Reset");
	vbox.pack_start(btn_reset);
	btn_reset.signal_clicked().connect(sigc::mem_fun(*this, &E3ChartAnalyzer::reset));

	btn_add_special.set_label("Add special");
	vbox.pack_end(btn_add_special);
	btn_add_special.signal_clicked().connect(sigc::mem_fun(*this, &E3ChartAnalyzer::on_add_special_clicked));

	this->add(vbox);
}

E3ChartAnalyzer::~E3ChartAnalyzer() {
	for(int i=0; i<charts.size(); i++)
		delete charts[i];
	delete config;
}

int E3ChartAnalyzer::getStatesCount(){
	return states_count;
}

void E3ChartAnalyzer::reset(){
	states_count = 0;
	for(int i=0; i<charts.size(); i++)
		charts[i]->reset();
}

void E3ChartAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(state);
		assert(msg);
	const google::protobuf::Message* d_msg = dynamic_cast<const google::protobuf::Message*>(d_state);
		assert(d_msg);

	pb::E4Special spec_msg;
	fill_spec_msg(state, d_state, &spec_msg);

	for(int i=0; i<charts.size(); i++){
		if(!chart_special_flags[i])
			charts[i]->processMessage(msg, d_msg, time);
		else
			charts[i]->processMessage(&spec_msg, NULL, time);
	}
}

void E3ChartAnalyzer::addChart(MessageChart* chart){
	charts.push_back(chart);
	chart_special_flags.push_back(false);
	vbox.pack_end(*chart, false, false, 20);
}

void E3ChartAnalyzer::addSpecial(MessageChart* chart){
	charts.push_back(chart);
	chart_special_flags.push_back(true);
	vbox.pack_end(*chart, false, false, 20);
}

void E3ChartAnalyzer::on_add_clicked(){
	const google::protobuf::Message* msg = new E3State();
	ChartAddDialog* dialog = new ChartAddDialog(msg, true);
	dialog->signal_cancel.connect(
			sigc::bind(
					sigc::mem_fun(*this, &E3ChartAnalyzer::on_dialog_cancel),
					dialog
			)
	);
	dialog->signal_ok.connect(
			sigc::bind(
					sigc::mem_fun(*this, &E3ChartAnalyzer::on_dialog_add_ok),
					dialog
			)
	);

	dialog->show_all();
}

void E3ChartAnalyzer::on_add_special_clicked(){
	const google::protobuf::Message* msg = new pb::E3Special();
	ChartAddDialog* dialog = new ChartAddDialog(msg, false);
	dialog->signal_cancel.connect(
			sigc::bind(
					sigc::mem_fun(*this, &E3ChartAnalyzer::on_dialog_cancel),
					dialog
			)
	);
	dialog->signal_ok.connect(
			sigc::bind(
					sigc::mem_fun(*this, &E3ChartAnalyzer::on_dialog_add_special_ok),
					dialog
			)
	);

	dialog->show_all();
}

void E3ChartAnalyzer::on_dialog_add_ok(ChartAddDialog* dialog){
	MessageChart* chart = new MessageChart(dialog->vars, NULL);
	addChart(chart);
	delete dialog;
}

void E3ChartAnalyzer::on_dialog_add_special_ok(ChartAddDialog* dialog){
	MessageChart* chart = new MessageChart(dialog->vars, NULL);
	addSpecial(chart);
	delete dialog;
}

void E3ChartAnalyzer::on_dialog_cancel(ChartAddDialog* dialog){
	delete dialog;
}

void E3ChartAnalyzer::on_del_chart_clicked(const MessageChart* chart){
	assert(chart);
	for(int i=0; i<charts.size(); i++){
		if(charts[i] == chart){
			delete charts[i];
			charts.erase(charts.begin()+i);
			return;
		}// if
	}// for
}

void E3ChartAnalyzer::fill_spec_msg(const OdeState* state, const OdeState* d_state, pb::E4Special* spec_msg){
	const E3State* estate = dynamic_cast<const E3State*>(state);
	const E3State* dstate = dynamic_cast<const E3State*>(d_state);
	const E3Config* config = dynamic_cast<const E3Config*>(this->config);

	int N = estate->particles_size();

	double sum_a_2 = 0;
	double sum_eta = 0;
	double Na = 0, Nb = 0;			// da/dt>0 and <0
	double Ia = 0.0, Ib = 0.0;	// sum da/dt
	double Wa = 0.0, Wb = 0.0;	// sum a^2
	for(int i=0; i<N; i++){
		E3State::Particles p = estate->particles(i);
		E3State::Particles dp = dstate->particles(i);

		sum_a_2 += p.a()*p.a();
		sum_eta += p.eta();

		if(dp.a() > 0.0){
			Na++;
			Ia += dp.a();
			Wa += p.a()*p.a();
		}// Na
		else{
			Nb++;
			Ib += dp.a();
			Wb += p.a()*p.a();
		}// Nb
	}

	Wa/=N; Wb/=N;
	Na/=N; Nb/=N;

	spec_msg->set_e_2(estate->e()*estate->e());
	spec_msg->set_aver_a_2(sum_a_2/estate->particles_size());

	spec_msg->set_na(Na);
	spec_msg->set_nb(Nb);
	spec_msg->set_wa(Wa);
	spec_msg->set_wb(Wb);

	spec_msg->set_n(Na+Nb);
	spec_msg->set_m(Na-Nb);
}

