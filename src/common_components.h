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

#include <../from_protoc/model_e4mm.pb.h>
#include <google/protobuf/message.h>

#include <gtkmm/builder.h>
#include <gtkmm/grid.h>
#include <gtkmm/entry.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/socket.h>

#include <vector>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace google::protobuf;

class EXPetscSolverConfig: public pb::EXPetscSolverConfig, public OdeSolverConfig{
public:
	virtual OdeSolverConfig* clone() const {
		EXPetscSolverConfig* ret = new EXPetscSolverConfig();
		ret->MergeFrom(*this);
		return ret;
	}
	EXPetscSolverConfig();
};

template<class AllModel, class ESolution>
class EXPetscSolver: public OdeSolver{
public:
	virtual const char* ts_path() = 0;

public:
	EXPetscSolver(const EXPetscSolverConfig*, const OdeConfig*, const OdeState*);
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
	OdeConfig* pconfig;				// problem config
	EXPetscSolverConfig* sconfig;		// solver config
	OdeState* state;
	OdeState* d_state;

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

class EXPetscSolverConfigWidget: public OdeSolverConfigWidget{
private:
	EXPetscSolverConfig* config;

	Gtk::Entry *entry_atol, *entry_rtol, *entry_step;
	Glib::RefPtr<Gtk::Adjustment> adj_n_cores;

public:
	EXPetscSolverConfigWidget(const EXPetscSolverConfig* config = NULL);
	virtual const OdeSolverConfig* getConfig();
	virtual void loadConfig(const OdeSolverConfig* config);

	static std::string getDisplayName(){
		return "PETSc solver for EX config widget";
	}
private:
	void widget_to_config();
	void config_to_widget();
};

class MessageChart: public Gtk::Bin{
private:
	Gnuplot* gnuplot;
	double last_time;
	google::protobuf::Message *last_msg, *last_d_msg;

	std::vector<std::string> vars;

	Gtk::Label label;	// with title
public:
	MessageChart(const std::vector<std::string>& vars, const std::string& x_var, Gtk::Container* parent = NULL);
	void processMessage(const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time);
	~MessageChart(){
		delete gnuplot;
		delete last_msg;
		delete last_d_msg;
	}

	sigc::signal<void> signal_closed;

//	void setXAxisTime(){
//		gnuplot->setXAxisTime();
//		update_title();
//	}
//	void setXAxisVar(const std::string& var){
//		gnuplot->setXAxisVar(var);
//		update_title();
//	}
	void setStyle(Gnuplot::style_enum style){
		gnuplot->setStyle(style);
	}
	bool getXAxisTime() const {
		return gnuplot->getXAxisTime();
	}
	std::string getXAxisVar() const{
		return gnuplot->getXAxisVar();
	}
	bool getPolar() const {
		return gnuplot->getPolar();
	}
	void setPolar(bool p){
		gnuplot->setPolar(p);
		update_title();
	}
	bool getParametric() const {
		return gnuplot->getParametric();
	}
	void setParametric(bool p){
		gnuplot->setParametric(p);
		if(p)
			gnuplot->setStyle(Gnuplot::STYLE_LINES);
		else if(vars[0].find('.')!=std::string::npos)
			gnuplot->setStyle(Gnuplot::STYLE_POINTS);
		update_title();
	}
	void reset(){
		gnuplot->reset();
	}
	void setXRange(double a, double b){
		gnuplot->setXRange(a, b);
	}
	void setYRange(double a, double b){
		gnuplot->setYRange(a, b);
	}

private:
	void update_title();

	void on_save_clicked();
	void on_del_chart_clicked();
	void on_writeback_clicked();
	void on_restore_clicked();
};


class ChartAddDialog: public Gtk::Window{
private:
	Gtk::TreeView *treeview1, *treeview2, *treeview3;		// for vars, derivatives and expressions
	Glib::RefPtr<Gtk::ListStore> store1, store2, store3;
	Gtk::Button *btn_plus, *btn_minus;
	Gtk::Button *btn_ok, *btn_cancel;
	Gtk::CheckButton *check_polar, *check_parametric;

	const google::protobuf::Message* msg;

public:

	sigc::signal<void> signal_ok, signal_cancel;

	// results
	std::vector<std::string> vars;
	std::string x_axis_var;
	bool polar;
	bool parametric;

	ChartAddDialog(const google::protobuf::Message* msg, bool show_derivatives = false);

private:
	void on_ok_clicked();
	void on_cancel_clicked();
	void on_use_clicked(const Glib::ustring& path, Glib::RefPtr<Gtk::ListStore> store);
	void on_x_clicked(const Glib::ustring& path, Glib::RefPtr<Gtk::ListStore> store);
	void on_expr_edited(const Glib::ustring& path,  const Glib::ustring& new_text);
	void on_plus_clicked();
	void on_minus_clicked();
};// ChartAddDialog

class EXChartAnalyzer: public OdeAnalyzerWidget {
protected:
	std::vector<MessageChart*> charts;
	int states_count;

	Gtk::VBox vbox;
	Gtk::Button btn_add;
	Gtk::Button btn_reset;

	const OdeConfig* config;
public:
	EXChartAnalyzer(const OdeConfig* config);
	virtual void loadConfig(const OdeConfig* config){}
	virtual void reset();
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual int getStatesCount();
	virtual ~EXChartAnalyzer();

	static std::string getDisplayName(){
		return "customizable chart for E4";
	}

	virtual void addChart(MessageChart* chart);

protected:
	// used in add button handler for creating message to edit it
	virtual ::google::protobuf::Message* new_state() = 0;

	void on_add_clicked();
	void on_del_chart_clicked(const MessageChart* chart);
	void on_dialog_add_ok(ChartAddDialog* dialog);
	void on_dialog_cancel(ChartAddDialog* dialog);
};

template<class AllModel, class ESolution>
EXPetscSolver<AllModel, ESolution>::EXPetscSolver(const EXPetscSolverConfig* scfg, const OdeConfig* pcfg, const OdeState* init_state){
	time_passed = 0;
	steps_passed = 0;

	pconfig = pcfg->clone();
	sconfig = new EXPetscSolverConfig(*scfg);
	state = init_state->clone();
	// TODO Here ws just new State. Make virtual New function?
	d_state = init_state->clone();
}

template<class AllModel, class ESolution>
EXPetscSolver<AllModel, ESolution>::~EXPetscSolver(){
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
template<class AllModel, class ESolution>
void EXPetscSolver<AllModel, ESolution>::run(int steps, double time, bool use_step){
//	printf("run started\n");
//	fflush(stdout);

	::google::protobuf::Message* pconfig = dynamic_cast<::google::protobuf::Message*>(this->pconfig);
	::google::protobuf::Message* state = dynamic_cast<::google::protobuf::Message*>(this->state);

	static int run_cnt = 0;
	run_cnt++;

	int n_cores = 1;
	if(this->sconfig->has_n_cores())
		n_cores = this->sconfig->n_cores();
	std::ostringstream cmd_stream;
//	cmd_stream << "mpiexec -n "<< n_cores << " --host 192.168.0.101 ./Debug/ts3";
//	cmd_stream << "mpiexec -n "<< n_cores << " --host 10.0.0.205 /home/dimalit/workspace/ts3/Debug/ts3";
	cmd_stream << "mpiexec -n "<< n_cores << ts_path();// << " -info info.log";

	std::string cmd = cmd_stream.str();
	if(use_step)
		cmd += " use_step";
	int rfd, wfd;
	child = rpc_call(cmd.c_str(), &rfd, &wfd);
	rf = fdopen(rfd, "rb");
	wf = fdopen(wfd, "wb");

//	int tmp = open("tmp", O_WRONLY | O_CREAT, 0664);
//	state->PrintDebugString();
//	wf = fopen("all.tmp", "wb");


	// TODO: send them as two separate messages - and remove AllModel!!
	AllModel all;
	all.mutable_sconfig()->CopyFrom(*sconfig);
	all.mutable_pconfig()->CopyFrom(*pconfig);
	all.mutable_state()->CopyFrom(*state);

	int size = all.ByteSize();

	int ok;
	ok = fwrite(&size, sizeof(size), 1, wf);
		assert(ok == 1);

	fflush(wf);
	all.SerializeToFileDescriptor(fileno(wf));

//	all.PrintDebugString();

	ok = fwrite(&steps, sizeof(steps), 1, wf);
		assert(ok == 1);
	ok = fwrite(&time, sizeof(time), 1, wf);
		assert(ok == 1);
	fflush(wf);
//exit(1);
	if(!use_step){		// just final step
		bool res = read_results();
			assert(res==true);		// last
		waitpid(child, 0, 0);
		fclose(rf); rf = NULL;
		fclose(wf); wf = NULL;
	}
}

template<class AllModel, class ESolution>
bool EXPetscSolver<AllModel, ESolution>::step(){
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

template<class AllModel, class ESolution>
bool EXPetscSolver<AllModel, ESolution>::read_results(){
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

	// TODO: unpair this to two separate messages too!!
	ESolution sol;
	extern void parse_with_prefix(google::protobuf::Message& msg, FILE* fp);
	parse_with_prefix(sol, rf);

//	sol.state().PrintDebugString();
//	fflush(stdout);


	::google::protobuf::Message* state = dynamic_cast<::google::protobuf::Message*>(this->state);
	::google::protobuf::Message* d_state = dynamic_cast<::google::protobuf::Message*>(this->d_state);

	state->CopyFrom(sol.state());
	if(sol.has_d_state())
		d_state->CopyFrom(sol.d_state());
	else
		this->d_state = NULL;

//	state->PrintDebugString();
//	d_state->PrintDebugString();

	return true;
}

template<class AllModel, class ESolution>
void EXPetscSolver<AllModel, ESolution>::finish(){
	fputc('f', wf);
	fflush(wf);
}

#endif /* COMMON_COMPONENTS_H_ */
