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

#define UI_FILE "charts_add.glade"
#define UI_FILE_STATE "src/models/e4/e4_state.glade"
#define UI_FILE_PETSC_SOLVER "src/models/e4/e4_petsc_solver.glade"

class EXPetscSolverConfig: public pb::EXPetscSolverConfig, public OdeSolverConfig{
public:
	virtual OdeSolverConfig* clone() const {
		EXPetscSolverConfig* ret = new EXPetscSolverConfig();
		ret->MergeFrom(*this);
		return ret;
	}
	EXPetscSolverConfig();
};

class EXPetscSolver: public OdeSolver{
public:
	static const char* ts_path;

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
		return "PETSc solver for E4 config widget";
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
	MessageChart(const std::vector<std::string>& vars, Gtk::Container* parent = NULL);
	void processMessage(const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time);
	~MessageChart(){
		delete gnuplot;
		delete last_msg;
		delete last_d_msg;
	}

	sigc::signal<void> signal_closed;

	void setXAxisTime(){
		gnuplot->setXAxisTime();
		update_title();
	}
	void setXAxisVar(const std::string& var){
		gnuplot->setXAxisVar(var);
		update_title();
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
	Gtk::CheckButton *check_polar;

	const google::protobuf::Message* msg;

public:

	sigc::signal<void> signal_ok, signal_cancel;

	// results
	std::vector<std::string> vars;
	std::string x_axis_var;
	bool polar = check_polar->get_active();

	ChartAddDialog(const google::protobuf::Message* msg, bool show_derivatives = false){
		assert(msg);
		this->msg = msg;
		this->set_modal(true);

		Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE);

		Gtk::Widget* root;
		b->get_widget("root", root);

		b->get_widget("treeview1", treeview1);
		b->get_widget("treeview2", treeview2);
		b->get_widget("treeview3", treeview3);
		b->get_widget("btn_plus", btn_plus);
		b->get_widget("btn_minus", btn_minus);
		b->get_widget("btn_ok", btn_ok);
		b->get_widget("btn_cancel", btn_cancel);
		b->get_widget("check_polar", check_polar);

		store1 = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(b->get_object("liststore1"));
		store2 = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(b->get_object("liststore2"));
		store3 = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(b->get_object("liststore3"));

		Gtk::CellRendererToggle *cr;
		cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview1->get_column(0)->get_first_cell());
			cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_use_clicked), store1));
		cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview1->get_column(2)->get_first_cell());
			cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_x_clicked), store1));
		cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview2->get_column(0)->get_first_cell());
			cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_use_clicked), store2));
		cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview2->get_column(2)->get_first_cell());
			cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_x_clicked), store2));
		cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview3->get_column(0)->get_first_cell());
			cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_use_clicked), store3));
		cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview3->get_column(2)->get_first_cell());
			cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_x_clicked), store3));

		Gtk::CellRendererText *crt = dynamic_cast<Gtk::CellRendererText*>(treeview3->get_column(1)->get_first_cell());
			crt->signal_edited().connect(sigc::mem_fun(*this, &ChartAddDialog::on_expr_edited));

		btn_plus->signal_clicked().connect(sigc::mem_fun(this, &ChartAddDialog::on_plus_clicked));
		btn_minus->signal_clicked().connect(sigc::mem_fun(this, &ChartAddDialog::on_minus_clicked));

		btn_ok->signal_clicked().connect(sigc::mem_fun(this, &ChartAddDialog::on_ok_clicked));
		btn_cancel->signal_clicked().connect(sigc::mem_fun(this, &ChartAddDialog::on_cancel_clicked));

		this->add(*root);

		// now add state's variables to the table
		store1->clear();
		store2->clear();
		store3->clear();
		const Descriptor* desc = msg->GetDescriptor();
		const Reflection* refl = msg->GetReflection();

		for(int i=0; i<desc->field_count(); i++){
			const FieldDescriptor* fd = desc->field(i);
			// add simple
			if(fd->type() == FieldDescriptor::TYPE_DOUBLE){
				Gtk::ListStore::iterator it = store1->append();
				it->set_value(0, false);
				it->set_value(1, Glib::ustring(fd->name()));
				it->set_value(2, false);

				it = store2->append();
				it->set_value(0, false);
				it->set_value(1, Glib::ustring(fd->name()));
				it->set_value(2, false);
			}
			// add message
			else if(fd->type() == FieldDescriptor::TYPE_MESSAGE || fd->type() == FieldDescriptor::TYPE_GROUP){
				const Message& m2 = fd->is_repeated() ? refl->GetRepeatedMessage(*msg, fd, 0) : refl->GetMessage(*msg, fd);

				const Descriptor* d2 = m2.GetDescriptor();
				for(int i=0; i<d2->field_count(); i++){
						const FieldDescriptor* fd2 = d2->field(i);
						if(fd2->type() != FieldDescriptor::TYPE_DOUBLE)
							continue;

						Gtk::ListStore::iterator it = store1->append();
						it->set_value(0, false);
						it->set_value(1, Glib::ustring(fd->name())+"."+Glib::ustring(fd2->name()));
						it->set_value(2, false);

						it = store2->append();
						it->set_value(0, false);
						it->set_value(1, Glib::ustring(fd->name())+"."+Glib::ustring(fd2->name()));
						it->set_value(2, false);
				}// for
			}// else
		}// for
	}

private:
	void on_ok_clicked(){
		vars.clear();
		x_axis_var = "";
		polar = check_polar->get_active();

		// check all repeated or all non-repeated
		bool has_repeated = false, has_non_repeated = false;

		// list1
		Gtk::ListStore::Children children = store1->children();
		for(Gtk::ListStore::const_iterator i = children.begin(); i!=children.end(); ++i){
			// parse added vars
			bool use;
			i->get_value(0, use);
			if(use){
				Glib::ustring us;
				i->get_value(1, us);
				vars.push_back(us.raw());

				if(us.raw().find('.') == std::string::npos)
					has_non_repeated = true;
				else
					has_repeated = true;
			}
			// parse x var
			bool as_x;
			i->get_value(2, as_x);
			if(as_x){
				Glib::ustring us;
				i->get_value(1, us);
				x_axis_var = us.raw();
			}// if as x
		}// for

		// list2
		children = store2->children();
		for(Gtk::ListStore::const_iterator i = children.begin(); i!=children.end(); ++i){
			// parse added vars
			bool use;
			i->get_value(0, use);
			if(use){
				Glib::ustring us;
				i->get_value(1, us);
				vars.push_back(us.raw()+'\'');

				if(us.raw().find('.') == std::string::npos)
					has_non_repeated = true;
				else
					has_repeated = true;
			}
			// parse x var
			bool as_x;
			i->get_value(2, as_x);
			if(as_x){
				Glib::ustring us;
				i->get_value(1, us);
				x_axis_var = us.raw()+'\'';
			}// if as x
		}// for

		// list3
		children = store3->children();
		for(Gtk::ListStore::const_iterator i = children.begin(); i!=children.end(); ++i){
			// parse added vars
			bool use;
			i->get_value(0, use);
			if(use){
				Glib::ustring us;
				i->get_value(1, us);
				vars.push_back(us.raw());

				if(us.raw().find('.') == std::string::npos)
					has_non_repeated = true;
				else
					has_repeated = true;
			}
			// parse x var
			bool as_x;
			i->get_value(2, as_x);
			if(as_x){
				Glib::ustring us;
				i->get_value(1, us);
				x_axis_var = us.raw();
			}// if as x
		}// for

		if(x_axis_var.find('.') != std::string::npos)
			has_repeated = true;
		else if(!x_axis_var.empty())
			has_non_repeated = true;

		// must have at least something
		if(vars.size() == 0){
			Gtk::MessageDialog dlg("ERROR: Please select vars to add to the chart!", false, Gtk::MessageType::MESSAGE_ERROR, Gtk::ButtonsType::BUTTONS_OK, true);
			dlg.run();
			return;
		}

		// all must be either repeated or non-repeated!
		if(has_repeated && has_non_repeated){
			Gtk::MessageDialog dlg("ERROR: Vars must be either all repeated or all non-repeated!", false, Gtk::MessageType::MESSAGE_ERROR, Gtk::ButtonsType::BUTTONS_OK, true);
			dlg.run();
			return;
		}

		//parent->addChart(msg, vars, x_axis_var, NULL, polar);
		//this->hide();
		//delete this;

		signal_ok.emit();
	}
	void on_cancel_clicked(){
		//this->hide();
		//delete this;		// XXX: can we do so?
		signal_cancel.emit();
	}
	void on_use_clicked(const Glib::ustring& path, Glib::RefPtr<Gtk::ListStore> store){
		Gtk::ListStore::iterator it = store->get_iter(path);
		bool val;
		it->get_value(0, val);
		it->set_value(0, !val);
	}
	void on_x_clicked(const Glib::ustring& path, Glib::RefPtr<Gtk::ListStore> store){
		// uncheck if checked
		Gtk::ListStore::iterator cur = store->get_iter(path);
		bool checked; cur->get_value(2, checked);
		if(checked){
			cur->set_value(2, false);
			return;
		}

		// clear all
		for(Gtk::ListStore::iterator it = store1->children().begin(); it != store1->children().end(); ++it){
			it->set_value(2, false);
		}
		for(Gtk::ListStore::iterator it = store2->children().begin(); it != store2->children().end(); ++it){
			it->set_value(2, false);
		}
		for(Gtk::ListStore::iterator it = store3->children().begin(); it != store3->children().end(); ++it){
			it->set_value(2, false);
		}

		// check
		cur->set_value(2, true);
	}

	void on_expr_edited(const Glib::ustring& path,  const Glib::ustring& new_text){
		Gtk::ListStore::iterator it = store3->get_iter(path);
		it->set_value(1, new_text);
	}

	void on_plus_clicked(){
		store3->append();
	}
	void on_minus_clicked(){
		if(treeview3->get_selection()->count_selected_rows()==1){
			Gtk::ListStore::iterator it = treeview3->get_selection()->get_selected();
			store3->erase(it);
		}
	}
};// ChartAddDialog

#endif /* COMMON_COMPONENTS_H_ */
