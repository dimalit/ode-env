/*
 * e3_analyzers.cpp
 *
 *  Created on: Aug 22, 2014
 *      Author: dimalit
 */

#include "analyzers_e42m.h"

#include <cassert>

#define UI_FILE "src/models/e42m/e42m_conservation.glade"

// TODO: make Gnuplotting" functionality apart and use it to draw energy chars!
E42mConservationAnalyzer::E42mConservationAnalyzer(const E42mConfig* config){
	states_count = 0;

	this->config = new E42mConfig(*config);

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

void E42mConservationAnalyzer::loadConfig(const OdeConfig* config){
	const E42mConfig* econfig = dynamic_cast<const E42mConfig*>(config);
		assert(econfig);

	this->config = new E42mConfig(*econfig);
}

void E42mConservationAnalyzer::reset(){
	states_count = 0;
}
void E42mConservationAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const E42mState* estate = dynamic_cast<const E42mState*>(state);
		assert(estate);

	liststore1->clear();

	double sum_a_2 = 0;
	double sum_x = 0, sum_y = 0;

	for(int i=0; i<estate->particles_size(); i++){
		E42mState::Particles p = estate->particles(i);

		Gtk::ListStore::iterator it = liststore1->append();
		it->set_value(0, p.a());
		it->set_value(1, p.psi());
		it->set_value(2, p.z());
		it->set_value(3, p.delta());

		sum_a_2 += p.a()*p.a();
		sum_x += p.a()*cos(p.psi()-2*M_PI*p.z());
		sum_y += p.a()*sin(p.psi()-2*M_PI*p.z());
	}

	std::ostringstream buf;
	buf.setf( std::ios::fixed, std:: ios::floatfield );
	buf.precision(10);

	buf.str("");
	buf << estate->e_p()*estate->e_p() + estate->e_m()*estate->e_m() + 1.0/config->n()*sum_a_2;
	entry_int1->set_text(buf.str());

	buf.str("");
	buf << estate->e_p();
	entry_e_p->set_text(buf.str());

	buf.str("");
	buf << estate->phi_p();
	entry_phi_p->set_text(buf.str());

	buf.str("");
	buf << estate->e_m();
	entry_e_m->set_text(buf.str());

	buf.str("");
	buf << estate->phi_m();
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

int E42mConservationAnalyzer::getStatesCount(){
	return states_count;
}

E42mConservationAnalyzer::~E42mConservationAnalyzer(){
	delete config;
}

E42mChartAnalyzer::E42mChartAnalyzer(const OdeConfig* config):EXChartAnalyzer(config) {
	btn_add_special.set_label("Add special");
	vbox.pack_end(btn_add_special);
	btn_add_special.signal_clicked().connect(sigc::mem_fun(*this, &E42mChartAnalyzer::on_add_special_clicked));
}

E42mChartAnalyzer::~E42mChartAnalyzer() {

}

void E42mChartAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(state);
		assert(msg);
	const google::protobuf::Message* d_msg = dynamic_cast<const google::protobuf::Message*>(d_state);
		assert(d_msg);

	pb::E42mSpecial spec_msg;
	fill_spec_msg(state, d_state, &spec_msg);

	for(int i=0; i<charts.size(); i++){
		if(!chart_special_flags[i])
			charts[i]->processMessage(msg, d_msg, time);
		else
			charts[i]->processMessage(&spec_msg, NULL, time);
	}
}

void E42mChartAnalyzer::addChart(MessageChart* chart){
	EXChartAnalyzer::addChart(chart);
	chart_special_flags.push_back(false);
}

void E42mChartAnalyzer::addSpecial(MessageChart* chart){
	charts.push_back(chart);
	chart_special_flags.push_back(true);
	vbox.pack_end(*chart, false, false, 1);
	this->show_all();
}

void E42mChartAnalyzer::on_add_special_clicked(){
	const google::protobuf::Message* msg = new pb::E42mSpecial();
	ChartAddDialog* dialog = new ChartAddDialog(msg, false);
	dialog->signal_cancel.connect(
			sigc::bind(
					sigc::mem_fun(*this, &E42mChartAnalyzer::on_dialog_cancel),
					dialog
			)
	);
	dialog->signal_ok.connect(
			sigc::bind(
					sigc::mem_fun(*this, &E42mChartAnalyzer::on_dialog_add_special_ok),
					dialog
			)
	);

	dialog->show_all();
}

void E42mChartAnalyzer::on_dialog_add_special_ok(ChartAddDialog* dialog){
	MessageChart* chart = new MessageChart(dialog->vars, NULL);
	if(dialog->x_axis_var!="")
		chart->setXAxisVar(dialog->x_axis_var);
	chart->setPolar(dialog->polar);
	addSpecial(chart);
	delete dialog;
}

void E42mChartAnalyzer::fill_spec_msg(const OdeState* state, const OdeState* d_state, pb::E42mSpecial* spec_msg){
	const E42mState* estate = dynamic_cast<const E42mState*>(state);
	const E42mState* dstate = dynamic_cast<const E42mState*>(d_state);
	const E42mConfig* config = dynamic_cast<const E42mConfig*>(this->config);

	int N = estate->particles_size();

	double sum_a_2 = 0;
	double Na = 0, Nb = 0;			// da/dt>0 and <0
	double Ia = 0.0, Ib = 0.0;	// sum da/dt
	double Wa = 0.0, Wb = 0.0;	// sum a^2
	for(int i=0; i<N; i++){
		E42mState::Particles p = estate->particles(i);
		E42mState::Particles dp = dstate->particles(i);

		sum_a_2 += p.a()*p.a();

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

	Na/=N; Nb/=N;
	Wa/=N; Wb/=N;

	spec_msg->set_e_p_2(estate->e_p()*estate->e_p());
	spec_msg->set_e_m_2(estate->e_m()*estate->e_m());
	spec_msg->set_aver_a_2(sum_a_2/estate->particles_size());
	spec_msg->set_int_e_a(estate->e_p()*estate->e_p() + estate->e_m()*estate->e_m() + 1.0/config->n()/estate->particles_size()*sum_a_2);

	spec_msg->set_na(Na);
	spec_msg->set_nb(Nb);
	spec_msg->set_ia(Ia);
	spec_msg->set_ib(Ib);
	spec_msg->set_wa(Wa);
	spec_msg->set_wb(Wb);

	spec_msg->set_ia_aver(Ia/Na);
	spec_msg->set_ib_aver(Ib/Nb);
	spec_msg->set_n(Na+Nb);
	spec_msg->set_m(Na-Nb);


	// compute max/min
	double min = std::numeric_limits<double>::infinity();
	double max = -std::numeric_limits<double>::infinity();

	for(int i=0; i<N; i++){
		E42mState::Particles p = estate->particles(i);
		double v = p.a()*p.a();
		if(v>max)
			max = v;
		if(v<min)
			min = v;
	}// for

	// fill histogram
	int hist[10] = {0};
	for(int i=0; i<N; i++){
		E42mState::Particles p = estate->particles(i);
		int bin = int((p.a()*p.a()-min)/(max-min)*10);
		if(bin>=10)
			bin=9;
		if(bin<0)
			bin=0;
		hist[bin]++;
	}// for

	spec_msg->clear_hist();
	for(int i=0; i<10; i++){
		spec_msg->add_hist();
		spec_msg->mutable_hist(i)->set_x(min+i/10.0*(max-min));
		spec_msg->mutable_hist(i)->set_y(hist[i]);
	}
}
