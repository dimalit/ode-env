#ifndef ANALYZERS_E42MC_H_
#define ANALYZERS_E42MC_H_

#include <gui_factories.h>

#include <gtkmm/entry.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/builder.h>
#include <models/e42mc/model_e42mc.h>

class E42mcConservationAnalyzer: public OdeAnalyzerWidget{
public:
	typedef E42mcConfig Config;
	typedef E42mcState State;
private:
	E42mcConfig* config;
	int states_count;
	time_t last_update;

	Gtk::TreeView *treeview1;
	Glib::RefPtr<Gtk::ListStore> liststore1;
	Gtk::Entry *entry_int1, *entry_int2;
	Gtk::Entry *entry_aver_x, *entry_aver_y;
	Gtk::Entry *entry_e_p, *entry_e_m, *entry_phi_p, *entry_phi_m;
	Gtk::Entry *entry_cm_r, *entry_cm_rp, *entry_cm_rm;
	Gtk::Entry *entry_aver_xp, *entry_aver_yp;
	Gtk::Entry *entry_aver_xm, *entry_aver_ym;
public:
	E42mcConservationAnalyzer(const E42mcConfig* config);
	virtual void loadConfig(const OdeConfig* config);
	virtual void reset();
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual int getStatesCount();
	virtual ~E42mcConservationAnalyzer();

	static std::string getDisplayName(){
		return "conservation analyzer for E42mc";
	}
};

class E42mcChartAnalyzer: public EXChartAnalyzer {
public:
	typedef E42mcConfig Config;
	typedef E42mcState State;

public:
	E42mcChartAnalyzer(const OdeConfig* config);
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual ~E42mcChartAnalyzer();

	static std::string getDisplayName(){
		return "customized for E42mc EXChartAnalyzer";
	}

	virtual void addChart(MessageChart* chart);
	virtual void addSpecial(MessageChart* chart);

private:
	virtual ::google::protobuf::Message* new_state(){ return new E42mcState();}
};

REGISTER_ANALYZER_WIDGET_CLASS(E42mcConservationAnalyzer)
REGISTER_ANALYZER_WIDGET_CLASS(E42mcChartAnalyzer)

#endif /* ANALYZERS_E42MC_H_ */
