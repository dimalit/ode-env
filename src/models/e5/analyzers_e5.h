#ifndef ANALYZERS_E5C_H_
#define ANALYZERS_E5_H_

#include <gui_factories.h>

#include <gtkmm/entry.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/builder.h>
#include <models/e5/model_e5.h>

class E5ConservationAnalyzer: public OdeAnalyzerWidget{
public:
	typedef E5Config Config;
	typedef E5State State;
private:
	E5Config* config;
	int states_count;
	time_t last_update;

	Gtk::TreeView *treeview1;
	Glib::RefPtr<Gtk::ListStore> liststore1;
	Gtk::Entry *entry_int1, *entry_int2;
	Gtk::Entry *entry_aver_x, *entry_aver_y;
	Gtk::Entry *entry_cm_r;
public:
	E5ConservationAnalyzer(const E5Config* config);
	virtual void loadConfig(const OdeConfig* config);
	virtual void reset();
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual int getStatesCount();
	virtual ~E5ConservationAnalyzer();

	static std::string getDisplayName(){
		return "conservation analyzer for E5";
	}
};

class E5ChartAnalyzer: public EXChartAnalyzer {
public:
	typedef E5Config Config;
	typedef E5State State;

public:
	E5ChartAnalyzer(const OdeConfig* config);
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual ~E5ChartAnalyzer();

	static std::string getDisplayName(){
		return "customized for E5 EXChartAnalyzer";
	}

	virtual void addChart(MessageChart* chart);
	virtual void addSpecial(MessageChart* chart);

private:
	virtual ::google::protobuf::Message* new_state(){ return new E5State();}
};

REGISTER_ANALYZER_WIDGET_CLASS(E5ConservationAnalyzer)
REGISTER_ANALYZER_WIDGET_CLASS(E5ChartAnalyzer)

#endif /* ANALYZERS_E5_H_ */
