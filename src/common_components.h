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

#include <rpc.h>

#include <../from_protoc/model_e4.pb.h>
#include <google/protobuf/message.h>

#include <gtkmm/builder.h>
#include <gtkmm/grid.h>
#include <gtkmm/entry.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>

#include <vector>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace google::protobuf;

#define UI_FILE_STATE "src/models/e4/e4_state.glade"
#define UI_FILE_PETSC_SOLVER "src/models/e4/e4_petsc_solver.glade"

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

class EXConfigWidget: public OdeConfigWidget{
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

template<class S>
class EXPetscSolverConfigWidget: public OdeSolverConfigWidget{
public:
	typedef S Solver;
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

/////////////////// IMPLEMENTATIONS ///////////////////////
// XXX: whis should read like E4...if we use more than 1 specialization?
template<class SC, class PC, class S>
const char* EXPetscSolver<SC, PC, S>::ts_path = " ../ts4/Debug/ts4";

template<class SC, class PC, class S>
EXPetscSolver<SC,PC,S>::EXPetscSolver(const SConfig* scfg, const PConfig* pcfg, const State* init_state){
	time_passed = 0;
	steps_passed = 0;

	pconfig = new PConfig(*pcfg);
	sconfig = new SConfig(*scfg);
	state = new State(*init_state);
	d_state = new State();
}

template<class SC, class PC, class S>
EXPetscSolver<SC,PC,S>::~EXPetscSolver(){
	if(wf){
		fputc('f', wf);
		fflush(wf);
	}
	delete state;
	delete sconfig;
	delete pconfig;
}

// TODO: create universal base class for PETSc solvers - so not to copypaste!
// TODO: 1 universal code from TS solving?! (not to write it again and again!?)
template<class SC, class PC, class S>
void EXPetscSolver<SC,PC,S>::run(int steps, double time, bool use_step){
//	printf("run started\n");
//	fflush(stdout);

	static int run_cnt = 0;
	run_cnt++;

	int n_cores = 1;
	if(this->sconfig->has_n_cores())
		n_cores = this->sconfig->n_cores();
	std::ostringstream cmd_stream;
//	cmd_stream << "mpiexec -n "<< n_cores << " --host 192.168.0.101 ./Debug/ts3";
//	cmd_stream << "mpiexec -n "<< n_cores << " --host 10.0.0.205 /home/dimalit/workspace/ts3/Debug/ts3";
	cmd_stream << "mpiexec -n "<< n_cores << ts_path;// << " -info info.log";

	std::string cmd = cmd_stream.str();
	if(use_step)
		cmd += " use_step";
	int rfd, wfd;
	child = rpc_call(cmd.c_str(), &rfd, &wfd);
	rf = fdopen(rfd, "rb");
	wf = fdopen(wfd, "wb");

//	int tmp = open("tmp", O_WRONLY | O_CREAT, 0664);
//	state->PrintDebugString();

	pb::E4Model all;
	all.mutable_sconfig()->CopyFrom(*sconfig);
	all.mutable_pconfig()->CopyFrom(*pconfig);
	all.mutable_state()->CopyFrom(*state);

	int size = all.ByteSize();

	int ok;
	ok = fwrite(&size, sizeof(size), 1, wf);
		assert(ok == 1);

	fflush(wf);
	all.SerializeToFileDescriptor(fileno(wf));

	ok = fwrite(&steps, sizeof(steps), 1, wf);
		assert(ok == 1);
	ok = fwrite(&time, sizeof(time), 1, wf);
		assert(ok == 1);
	fflush(wf);

	if(!use_step){		// just final step
		bool res = read_results();
			assert(res==true);		// last
		waitpid(child, 0, 0);
		fclose(rf); rf = NULL;
		fclose(wf); wf = NULL;
	}
}

template<class SC, class PC, class S>
bool EXPetscSolver<SC,PC,S>::step(){
	if(waitpid(child, 0, WNOHANG)!=0){
		fclose(rf); rf = NULL;
		fclose(wf); wf = NULL;
		return false;
	}

	fputc('s', wf);
	fflush(wf);

	if(!read_results()){
		// TODO: will it ever run? (see waitpid above)
		waitpid(child, 0, 0);		// was before read - here for tests
		fclose(rf); rf = NULL;
		fclose(wf); wf = NULL;
		return false;
	}
	return true;
}

template<class SC, class PC, class S>
bool EXPetscSolver<SC,PC,S>::read_results(){
	int ok;
	ok = fread(&steps_passed, sizeof(steps_passed), 1, rf);
	// TODO: read=0 no longer works with mpiexec (process isn't zombie)
	if(ok==0)
		return false;
	else
		assert(ok == 1);
	ok = fread(&time_passed, sizeof(time_passed), 1, rf);
		assert(ok == 1);

//	printf("%d %lf %s\n", steps_passed, time_passed, sconfig->model().c_str());
//	fflush(stdout);

	pb::E4Solution sol;
	extern void parse_with_prefix(google::protobuf::Message& msg, FILE* fp);
	parse_with_prefix(sol, rf);

//	sol.state().PrintDebugString();
//	fflush(stdout);

	state->CopyFrom(sol.state());
	d_state->CopyFrom(sol.d_state());
	return true;
}

template<class SC, class PC, class S>
void EXPetscSolver<SC,PC,S>::finish(){
	fputc('f', wf);
	fflush(wf);
}

template<class S>
EXPetscSolverConfigWidget<S>::EXPetscSolverConfigWidget(const EXPetscSolverConfig* config){
	if(config)
		this->config = new EXPetscSolverConfig(*config);
	else
		this->config = new EXPetscSolverConfig();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_PETSC_SOLVER);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_atol", entry_atol);
	b->get_widget("entry_rtol", entry_rtol);
	b->get_widget("entry_step", entry_step);
	adj_n_cores = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(b->get_object("adj_n_cores"));
	adj_n_cores->set_value(4);

	this->add(*root);

	config_to_widget();
}

template<class S>
const OdeSolverConfig* EXPetscSolverConfigWidget<S>::getConfig(){
	widget_to_config();
	return config;
}

template<class S>
void EXPetscSolverConfigWidget<S>::loadConfig(const OdeSolverConfig* config){
	const EXPetscSolverConfig* econfig = dynamic_cast<const EXPetscSolverConfig*>(config);
		assert(econfig);
	delete this->config;
	this->config = new EXPetscSolverConfig(*econfig);
	config_to_widget();
}

template<class S>
void EXPetscSolverConfigWidget<S>::widget_to_config(){
	config->set_init_step(atof(entry_step->get_text().c_str()));
	config->set_atol(atof(entry_atol->get_text().c_str()));
	config->set_rtol(atof(entry_rtol->get_text().c_str()));
	config->set_n_cores(adj_n_cores->get_value());
}

template<class S>
void EXPetscSolverConfigWidget<S>::config_to_widget(){
	std::ostringstream buf;
	buf << config->init_step();
	entry_step->set_text(buf.str());

	buf.str("");
	buf << config->atol();
	entry_atol->set_text(buf.str());

	buf.str("");
	buf << config->rtol();
	entry_rtol->set_text(buf.str());
	adj_n_cores->set_value(config->n_cores());
}

//REGISTER_ANALYZER_WIDGET_CLASS(ChartAnalyzer)

#endif /* COMMON_COMPONENTS_H_ */
