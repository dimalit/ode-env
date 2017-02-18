/*
 * analyzers_e3.h
 *
 *  Created on: Aug 22, 2014
 *      Author: dimalit
 */

#ifndef ANALYZERS_E4_H_
#define ANALYZERS_E4_H_

#include "model_e4.h"

#include <gui_factories.h>

#include <gtkmm/entry.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/builder.h>

class E4ConservationAnalyzer: public OdeAnalyzerWidget{
public:
	typedef E4Config Config;
	typedef E4State State;
private:
	E4Config* config;
	int states_count;
	time_t last_update;

	Gtk::TreeView *treeview1;
	Glib::RefPtr<Gtk::ListStore> liststore1;
	Gtk::Entry *entry_int1, *entry_int2;
	Gtk::Entry *entry_aver_x, *entry_aver_y;
	Gtk::Entry *entry_e, *entry_phi, *entry_cm_r;
public:
	E4ConservationAnalyzer(const E4Config* config);
	virtual void loadConfig(const OdeConfig* config);
	virtual void reset();
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual int getStatesCount();
	virtual ~E4ConservationAnalyzer();

	static std::string getDisplayName(){
		return "conservation analyzer for E4";
	}
};

class E4ChartAnalyzer: public EXChartAnalyzer {
public:
	typedef E4Config Config;
	typedef E4State State;
private:
	std::vector<bool> chart_special_flags;
	Gtk::Button btn_add_special;

public:
	E4ChartAnalyzer(const OdeConfig* config);
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual ~E4ChartAnalyzer();

	static std::string getDisplayName(){
		return "customized for E4 EXChartAnalyzer";
	}

	virtual void addChart(MessageChart* chart);
	virtual void addSpecial(MessageChart* chart);

private:
	virtual ::google::protobuf::Message* new_state(){ return new E4State();}
	void fill_spec_msg(const OdeState* state, const OdeState* d_state, pb::E4Special* spec_msg);

	void on_add_special_clicked();
	void on_dialog_add_special_ok(ChartAddDialog* dialog);
};

REGISTER_ANALYZER_WIDGET_CLASS(E4ConservationAnalyzer)
REGISTER_ANALYZER_WIDGET_CLASS(E4ChartAnalyzer)

#endif /* ANALYZERS_E4_H_ */
