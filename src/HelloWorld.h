#ifndef GTKMM_EXAMPLE_HELLOWORLD_H
#define GTKMM_EXAMPLE_HELLOWORLD_H

#include "gui_e1.h"

#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>

class HelloWorld : public Gtk::Window
{

public:
  HelloWorld();
  virtual ~HelloWorld();

protected:
  // Signal handlers:
  void on_launch_clicked();
  void on_cancel_clicked();

  // Member widgets:
  Gtk::Button launch_button, cancel_button;
  OdeConfigWidget* config_widget;
  OdeStateWidget* state_widget;
  OdeSolverConfigWidget* solver_config_widget;

  Gtk::VBox vbox;
  Gtk::HButtonBox button_box;
};

#endif // GTKMM_EXAMPLE_HELLOWORLD_H
