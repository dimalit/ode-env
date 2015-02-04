/*
 * analyzers_e3.h
 *
 *  Created on: Aug 22, 2014
 *      Author: dimalit
 */

#ifndef ANALYZERS_E3_H_
#define ANALYZERS_E3_H_

#include "model_e3.h"
#include "gui_factories.h"

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

	Gtk::TreeView *treeview1;
	Glib::RefPtr<Gtk::ListStore> liststore1;
	Gtk::Entry *entry_i1, *entry_i2, *entry_i3;
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

REGISTER_ANALYZER_WIDGET_CLASS(E3ConservationAnalyzer)

#endif /* ANALYZERS_E3_H_ */
