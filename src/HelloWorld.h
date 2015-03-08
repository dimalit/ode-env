#ifndef GTKMM_EXAMPLE_HELLOWORLD_H
#define GTKMM_EXAMPLE_HELLOWORLD_H

#include "gui_e1.h"
#include "ChartAnalyzer.h"

#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>

#include <glibmm/main.h>
#include <glibmm/threads.h>

#include <auto_ptr.h>

// TODO: how to use sigc::trackable right?
class RunThread: public sigc::trackable{
private:
	OdeSolver* solver;
	Glib::Threads::Thread* thread;
	sigc::signal<void, const OdeState*, const OdeState*> m_signal_finished;

public:
	RunThread(OdeSolver*);
	virtual ~RunThread();
	void run(int steps, double time);
	sigc::signal<void, const OdeState*, const OdeState*> getSignalFinished() const {
		return m_signal_finished;
	}

private:
	int steps;
	double time;
	const OdeState* final_state;
	const OdeState* final_d_state;

	int fd[2];
	Glib::RefPtr<Glib::IOSource> iosource;

	void thread_func();
	bool on_event(Glib::IOCondition);
};

class HelloWorld : public Gtk::Window
{
private:
  std::string problem_name;
  OdeSolver* solver;
  RunThread* run_thread;

public:
  HelloWorld();
  virtual ~HelloWorld();

private:
  int total_steps;
  double total_time;
  void set_steps_and_time(int steps, double time);
  void add_steps_and_time(int steps, double time);

  bool computing;
  int steps;
  double time;
  void run_computing();
  void one_run_completed_cb(const OdeState* final_state, const OdeState* final_d_state);
  void stop_computing();

  std::auto_ptr<OdeState> saved_state, saved_dstate;			// for reset

  // Signal handlers:
  void on_config_changed();
  void on_state_changed();

  void on_forever_clicked();
  void on_step_clicked();
  void on_reset_clicked();
  void on_cancel_clicked();

  const OdeConfig* extract_config();
  const OdeState* extract_state();
  const OdeSolverConfig* extract_solver_config();

  // Member widgets:
  OdeConfigWidget* config_widget;
  OdeStateWidget* state_widget;
  OdeSolverConfigWidget* solver_config_widget;

  Gtk::Window win_analyzers;
  OdeAnalyzerWidget* analyzer_widget;
  ChartAnalyzer* chart_analyzer;

  Gtk::RadioButton *radio_time, *radio_steps;
  Gtk::Entry *entry_time, *entry_steps;
  Gtk::Label *label_time, *label_steps;

  Gtk::Button forever_button, cancel_button, step_button, reset_button;

  Gtk::VBox vbox;
  Gtk::HButtonBox button_box;
};

#endif // GTKMM_EXAMPLE_HELLOWORLD_H
