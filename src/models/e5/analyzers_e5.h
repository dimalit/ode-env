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
	bool auto_update;

	Gtk::TreeView *treeview1;
	Glib::RefPtr<Gtk::ListStore> liststore1;
	Gtk::Entry *entry_int1, *entry_int2;
	Gtk::Entry *entry_aver_x, *entry_aver_y;
	Gtk::Entry *entry_cm_r;
	Gtk::ToggleButton *button_update;

	void on_update_toggled();
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

class E5FieldAnalyzer: public OdeAnalyzerWidget{
public:
	typedef E5Config Config;
	typedef E5State State;
private:
	E5Config* config;
	int states_count;

	MessageChart field_chart;
	MessageChart center_chart;
	pb::E5FieldProfile profile_message;
	pb::E5CenterField center_message;

public:
	E5FieldAnalyzer(const E5Config* config);
	virtual void loadConfig(const OdeConfig* config);
	virtual void reset();
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual int getStatesCount(){
		return states_count;
	}
	virtual ~E5FieldAnalyzer();

	static std::string getDisplayName(){
		return "Field profile analyzer for E5";
	}
};

REGISTER_ANALYZER_WIDGET_CLASS(E5ConservationAnalyzer)
REGISTER_ANALYZER_WIDGET_CLASS(E5ChartAnalyzer)
REGISTER_ANALYZER_WIDGET_CLASS(E5FieldAnalyzer)

#endif /* ANALYZERS_E5_H_ */
