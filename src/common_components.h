/*
 * common_components.h
 *
 *  Created on: Dec 26, 2016
 *      Author: dimalit
 */

#ifndef COMMON_COMPONENTS_H_
#define COMMON_COMPONENTS_H_

#include "gui_interfaces.h"
#include "Gnuplot.h"

#include <google/protobuf/message.h>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <vector>

using namespace google::protobuf;

class EXPetscSolverConfig: public pb::EXPetscSolverConfig, public OdeSolverConfig{
public:
	EXPetscSolverConfig();
};

template<class SC, class PC, class S>
class EXPetscSolver: public OdeSolver{
public:
	typedef SC SConfig;
	typedef PC PConfig;
	typedef S State;

	static const char* ts_path;

public:
	EXPetscSolver(const SConfig*, const PConfig*, const State*);
	virtual ~EXPetscSolver();
	virtual void run(int steps, double time, bool use_steps = false);
	virtual bool step();
	virtual void finish();
	virtual double getTime() const {
		return time_passed;
	}
	virtual double getSteps() const {
		return steps_passed;
	}

	virtual const OdeState* getState() const {
		return state;
	}
	virtual const OdeState* getDState() const{
		return d_state;
	}

	static std::string getDisplayName(){
		return "PETSc RK solver for eX";
	}

private:
	PConfig* pconfig;				// problem config
	SConfig* sconfig;	// solver config
	State* state;
	State* d_state;

	FILE  *rf, *wf;
	pid_t child;

	double time_passed;
	int steps_passed;

private:
//	int read_simulation(FILE* fp, int m);
	bool read_results();
};

// monitors EXTERNAL Message - with no ownership!
class AbstractConfigWidget: public Gtk::Bin{
private:
	mutable Message* data;

	Gtk::Grid grid;
	mutable Gtk::Button button_apply;

	std::map<string, Gtk::Entry*> entry_map;

	sigc::signal<void> m_signal_changed;
public:
	AbstractConfigWidget(Message *msg = NULL);

	void setData(Message* msg);
	const Message* getData() const;
	void update(){
		config_to_widget();
	}

	// only fires when changed from GUI
	sigc::signal<void> signal_changed() const{
		return m_signal_changed;
	}

	virtual ~AbstractConfigWidget(){
		delete data;
	}

private:
	void construct_ui();
	void widget_to_config() const;
	void config_to_widget() const;
	void edit_anything_cb();
	void on_apply_cb();
};

template<class C>
class EXConfigWidget: public OdeConfigWidget{
//!!! TODO: remove this - in gui_interfaces
public:
	typedef C Config;
private:
	OdeConfig* config;

	AbstractConfigWidget cfg_widget;

public:
	EXConfigWidget(const OdeConfig* config = NULL);

	virtual const OdeConfig* getConfig() const;
	virtual void loadConfig(const OdeConfig* cfg);

	static std::string getDisplayName(){
		return "GTK+ widgets for EX";
	}

private:
	void on_changed();
};

class EXPetscSolverConfigWidget: public OdeSolverConfigWidget{
public:
	typedef E4PetscSolver Solver;
private:
	EXPetscSolverConfig* config;

	Gtk::Entry *entry_atol, *entry_rtol, *entry_step;
	Glib::RefPtr<Gtk::Adjustment> adj_n_cores;

public:
	EXPetscSolverConfigWidget(const EXPetscSolverConfig* config = NULL);
	virtual const OdeSolverConfig* getConfig();
	virtual void loadConfig(const OdeSolverConfig* config);

	static std::string getDisplayName(){
		return "PETSc solver for E4 config widget";
	}
private:
	void widget_to_config();
	void config_to_widget();
};

class ChartAnalyzer: public OdeAnalyzerWidget {
private:
	std::vector<Gnuplot*> plots;
	std::vector<bool> plot_special_flags;
	int states_count;

	Gtk::VBox vbox;
	Gtk::Button btn_add;
	Gtk::Button btn_reset;
	Gtk::Button btn_add_special;

	const OdeState *last_state, *last_d_state;
	double last_time;
	google::protobuf::Message *last_special;
	double last_special_time;
	const google::protobuf::Message* special_msg;			// for addition of special vars
public:
	ChartAnalyzer(const OdeConfig* config);
	virtual void loadConfig(const OdeConfig* config){}
	virtual void reset();
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual int getStatesCount();
	virtual ~ChartAnalyzer();

	void addChart(const google::protobuf::Message* msg, std::vector<std::string> vars, std::string x_axis_var="", Gtk::Container* parent=NULL, bool polar=false, double yrange=0.0);

	void addSpecial(const google::protobuf::Message*);
	void processSpecial(const google::protobuf::Message* msg, double time);

	static std::string getDisplayName(){
		return "customizable chart";
	}
private:
	void on_save_clicked(Gnuplot* ptr);
	void on_add_clicked();
	void on_del_chart_clicked(Gtk::Widget* w, const Gnuplot* ptr);
	void on_writeback_clicked(Gnuplot* ptr);
	void on_restore_clicked(Gnuplot* ptr);
	void on_add_special_clicked();
};

//REGISTER_ANALYZER_WIDGET_CLASS(ChartAnalyzer)

#endif /* COMMON_COMPONENTS_H_ */
