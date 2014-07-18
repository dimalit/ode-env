#ifndef GTKMM_EXAMPLE_HELLOWORLD_H
#define GTKMM_EXAMPLE_HELLOWORLD_H

#include "gui_e1.h"

#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>

#include <glibmm/main.h>
#include <glibmm/threads.h>

// TODO: how to use sigc::trackable right?
class RunThread: public sigc::trackable{
private:
	OdeSolver* solver;
	Glib::Threads::Thread* thread;
	sigc::signal<void, const OdeState*> m_signal_finished;

public:
	RunThread(OdeSolver*);
	virtual ~RunThread();
	void run(double time_or_steps, bool as_steps = false);
	sigc::signal<void, const OdeState*> getSignalFinished() const {
		return m_signal_finished;
	}

private:
	double time_or_steps;
	bool as_steps;
	const OdeState* final_state;

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
  bool computing;
  double time_or_steps;
  bool as_steps;
  void run_computing();
  void one_run_completed_cb(const OdeState* final_state);
  void stop_computing();

  // Signal handlers:
  void on_config_changed();
  void on_state_changed();

  void on_launch_clicked();
  void on_cancel_clicked();

  const OdeConfig* extract_config();
  const OdeState* extract_state();
  const OdeSolverConfig* extract_solver_config();

  // Member widgets:
  OdeConfigWidget* config_widget;
  OdeStateWidget* state_widget;
  OdeSolverConfigWidget* solver_config_widget;

  OdeAnalyzerWidget* analyzer_widget;

  Gtk::Window win_state;
  Gtk::RadioButton *radio_time, *radio_steps;
  Gtk::Entry *entry_time, *entry_steps;
  Gtk::Label *label_time, *label_steps;

  Gtk::Button launch_button, cancel_button;

  Gtk::VBox vbox;
  Gtk::HButtonBox button_box;
};

#endif // GTKMM_EXAMPLE_HELLOWORLD_H
