#ifndef GTKMM_EXAMPLE_HELLOWORLD_H
#define GTKMM_EXAMPLE_HELLOWORLD_H

#include "gui_e1.h"

#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>

class HelloWorld : public Gtk::Window
{
	std::string problem_name;

public:
  HelloWorld();
  virtual ~HelloWorld();

protected:
  // Signal handlers:
  void on_config_changed();
  void on_state_changed();

  void on_config_apply();
  void on_state_apply();

  void on_launch_clicked();
  void on_cancel_clicked();

  // Member widgets:
  OdeConfigWidget* config_widget;
  OdeStateWidget* state_widget;
  OdeSolverConfigWidget* solver_config_widget;

  OdeAnalyzerWidget* analyzer_widget;

  Gtk::Window win_state;
  Gtk::VBox win_state_vbox;
  Gtk::RadioButton *radio_time, *radio_steps;
  Gtk::Entry *entry_time, *entry_steps;
  Gtk::Label *label_time, *label_steps;

  Gtk::Button config_apply_button, state_apply_button;
  Gtk::Button launch_button, cancel_button;

  Gtk::VBox vbox;
  Gtk::HButtonBox button_box;
};

#endif // GTKMM_EXAMPLE_HELLOWORLD_H
