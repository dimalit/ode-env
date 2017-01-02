/*
 * analyzers_e3.h
 *
 *  Created on: Aug 22, 2014
 *      Author: dimalit
 */

#ifndef ANALYZERS_E3_H_
#define ANALYZERS_E3_H_

#include "model_e3.h"

#include <gui_factories.h>
#include <common_components.h>

#include "gtkmm/entry.h"
#include "gtkmm/treeview.h"
#include "gtkmm/liststore.h"
#include "gtkmm/builder.h"

class E3ConservationAnalyzer: public OdeAnalyzerWidget{
public:
	typedef E3Config Config;
	typedef E3State State;
private:
	E3Config* config;
	int states_count;
	time_t last_update;

	Gtk::TreeView *treeview1;
	Glib::RefPtr<Gtk::ListStore> liststore1;
	Gtk::Entry *entry_i1, *entry_i2, *entry_i3;
	Gtk::Entry *entry_e, *entry_phi;
public:
	E3ConservationAnalyzer(const E3Config* config);
	virtual void loadConfig(const OdeConfig* config);
	virtual void reset();
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual int getStatesCount();
	virtual ~E3ConservationAnalyzer();

	static std::string getDisplayName(){
		return "conservation analyzer for E3";
	}
};

class E3ChartAnalyzer: public OdeAnalyzerWidget {
public:
	typedef E3Config Config;
	typedef E3State State;
private:
	std::vector<MessageChart*> charts;
	std::vector<bool> chart_special_flags;
	int states_count;

	Gtk::VBox vbox;
	Gtk::Button btn_add;
	Gtk::Button btn_reset;
	Gtk::Button btn_add_special;

	const OdeConfig* config;
public:
	E3ChartAnalyzer(const OdeConfig* config);
	virtual void loadConfig(const OdeConfig* config){}
	virtual void reset();
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual int getStatesCount();
	virtual ~E3ChartAnalyzer();

	static std::string getDisplayName(){
		return "customizable chart for E4";
	}

	void addChart(MessageChart* chart);
	void addSpecial(MessageChart* chart);

private:
	void fill_spec_msg(const OdeState* state, const OdeState* d_state, pb::E3Special* spec_msg);

	void on_add_clicked();
	void on_add_special_clicked();
	void on_del_chart_clicked(const MessageChart* chart);
	void on_dialog_add_ok(ChartAddDialog* dialog);
	void on_dialog_add_special_ok(ChartAddDialog* dialog);
	void on_dialog_cancel(ChartAddDialog* dialog);
};

REGISTER_ANALYZER_WIDGET_CLASS(E3ConservationAnalyzer)
REGISTER_ANALYZER_WIDGET_CLASS(E3ChartAnalyzer)

#endif /* ANALYZERS_E3_H_ */
