/*
 * analyzers_e3.h
 *
 *  Created on: Aug 22, 2014
 *      Author: dimalit
 */

#ifndef ANALYZERS_E42M_H_
#define ANALYZERS_E42M_H_

#include "model_e42m.h"

#include <gui_factories.h>

#include <gtkmm/entry.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/builder.h>

class E42mConservationAnalyzer: public OdeAnalyzerWidget{
public:
	typedef E42mConfig Config;
	typedef E42mState State;
private:
	E42mConfig* config;
	int states_count;
	time_t last_update;

	Gtk::TreeView *treeview1;
	Glib::RefPtr<Gtk::ListStore> liststore1;
	Gtk::Entry *entry_int1, *entry_int2;
	Gtk::Entry *entry_aver_x, *entry_aver_y;
	Gtk::Entry *entry_e_p, *entry_e_m, *entry_phi_p, *entry_phi_m, *entry_cm_r;
public:
	E42mConservationAnalyzer(const E42mConfig* config);
	virtual void loadConfig(const OdeConfig* config);
	virtual void reset();
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual int getStatesCount();
	virtual ~E42mConservationAnalyzer();

	static std::string getDisplayName(){
		return "conservation analyzer for E42m";
	}
};

class E42mChartAnalyzer: public EXChartAnalyzer {
public:
	typedef E42mConfig Config;
	typedef E42mState State;
private:
	std::vector<bool> chart_special_flags;
	Gtk::Button btn_add_special;

public:
	E42mChartAnalyzer(const OdeConfig* config);
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual ~E42mChartAnalyzer();

	static std::string getDisplayName(){
		return "customized for E42m EXChartAnalyzer";
	}

	virtual void addChart(MessageChart* chart);
	virtual void addSpecial(MessageChart* chart);

private:
	virtual ::google::protobuf::Message* new_state(){ return new E42mState();}
	void fill_spec_msg(const OdeState* state, const OdeState* d_state, pb::E42mSpecial* spec_msg);

	void on_add_special_clicked();
	void on_dialog_add_special_ok(ChartAddDialog* dialog);
};

REGISTER_ANALYZER_WIDGET_CLASS(E42mConservationAnalyzer)
REGISTER_ANALYZER_WIDGET_CLASS(E42mChartAnalyzer)

#endif /* ANALYZERS_E42M_H_ */
