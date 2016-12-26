#ifndef GTKMM_EXAMPLE_HELLOWORLD_H
#define GTKMM_EXAMPLE_HELLOWORLD_H

#include "models/e1/gui_e1.h"
#include "models/e3/gui_e3.h"
#include "ChartAnalyzer.h"
#include "RunThread.h"

#include <gtkmm/button.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>

#include <glibmm/main.h>
#include <glibmm/threads.h>

#include <auto_ptr.h>

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
  int run_steps;
  double run_time;
  double last_refresh_time;
  void show_steps_and_time();

  bool computing;
  int steps;
  double time;
  void run_computing(bool use_step);
  void run_finished_cb();
  void run_stepped_cb();
  void stop_computing();

  // Signal handlers:
  void on_config_changed();
  void on_state_changed();
  void show_new_state();

  void on_forever_clicked();
  void on_step_clicked();
  void on_reset_clicked();
  void on_cancel_clicked();

  const OdeConfig* extract_config();
  const OdeState* extract_state();
  const OdeSolverConfig* extract_solver_config();
  void fill_spec_msg(pb::E3Special*);

  const OdeState* state;
  const OdeState* d_state;

  // for diagnostics
  double sum_ia, sum_ib;
  double sum_wa, sum_wb;

  // Member widgets:
  OdeConfigWidget* config_widget;
  OdeStateGeneratorWidget* generator_widget;
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
