/*
 * ChartAnalyzer.cpp
 *
 *  Created on: Jan 29, 2015
 *      Author: dimalit
 */

#include "ChartAnalyzer.h"
#include <google/protobuf/message.h>

#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/messagedialog.h>

#include <iostream>
#include <cstdio>

using namespace google::protobuf;

#define UI_FILE "charts_add.glade"

class ChartAddDialog: public Gtk::Window{
private:
	Gtk::TreeView *treeview1, *treeview2, *treeview3;		// for vars, derivatives and expressions
	Glib::RefPtr<Gtk::ListStore> store1, store2, store3;
	Gtk::Button *btn_plus, *btn_minus;
	Gtk::Button *btn_ok, *btn_cancel;
	Gtk::CheckButton *check_polar;
	ChartAnalyzer* parent;

	const google::protobuf::Message* msg;
public:
	ChartAddDialog(ChartAnalyzer* parent, const google::protobuf::Message* msg){
		assert(parent && msg);
		this->parent = parent;
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
		std::vector<std::string> vars;
		std::string x_axis_var;

		// check all repeated or all non-repeated
		bool has_repeated = false, has_non_repeated = false;

		bool polar = check_polar->get_active();

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

		parent->addChart(msg, vars, x_axis_var, polar);

		this->hide();
		delete this;
	}
	void on_cancel_clicked(){
		this->hide();
		delete this;		// XXX: can we do so?
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
};

ChartAnalyzer::ChartAnalyzer(const OdeConfig* config) {
	states_count = 0;
	last_state = NULL;
	last_d_state = NULL;
	special_msg = NULL;

	btn_add.set_label("Add chart");
	vbox.pack_start(btn_add);
	btn_add.signal_clicked().connect(sigc::mem_fun(*this, &ChartAnalyzer::on_add_clicked));

	btn_reset.set_label("Reset");
	vbox.pack_start(btn_reset);
	btn_reset.signal_clicked().connect(sigc::mem_fun(*this, &ChartAnalyzer::reset));

	last_state = last_d_state = NULL;
	last_special = NULL;

	this->add(vbox);
}

void ChartAnalyzer::addSpecial(const google::protobuf::Message* msg){
	assert(!this->special_msg);

	this->special_msg = msg;

	btn_add_special.set_label("Add special");
	vbox.pack_end(btn_add_special);
	btn_add_special.signal_clicked().connect(sigc::mem_fun(*this, &ChartAnalyzer::on_add_special_clicked));
}

ChartAnalyzer::~ChartAnalyzer() {
	for(int i=0; i<plots.size(); i++)
		delete plots[i];
	delete special_msg;
}

int ChartAnalyzer::getStatesCount(){
	return states_count;
}

void ChartAnalyzer::reset(){
	states_count = 0;
	for(int i=0; i<plots.size(); i++)
		plots[i]->reset();
}

void ChartAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	if(last_state){
		delete last_state; last_state = NULL;
	}
	if(last_d_state){
		delete last_d_state; last_d_state = NULL;
	}

	last_state = state->clone();
	if(d_state)
		last_d_state = d_state->clone();
	last_time = time;

	const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(state);
		assert(msg);
	const google::protobuf::Message* d_msg = dynamic_cast<const google::protobuf::Message*>(d_state);
		assert(d_msg);

	for(int i=0; i<plots.size(); i++){
		if(!plot_special_flags[i])
			plots[i]->processState(msg, d_msg, time);
	}
}

void ChartAnalyzer::processSpecial(const google::protobuf::Message* msg, double time){
	if(last_special){
		delete last_special; last_special = NULL;
	}
	last_special = msg->New();
	last_special->CopyFrom(*msg);

	last_special_time = time;

	for(int i=0; i<plots.size(); i++){
		if(plot_special_flags[i])
			plots[i]->processState(msg, NULL, time);
	}
}

void ChartAnalyzer::on_save_clicked(Gnuplot* ptr){
	assert(last_state);
	assert(last_d_state);

	std::string file = ptr->getTitle()+".csv";

	const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(last_state);
		assert(msg);
	const google::protobuf::Message* d_msg = dynamic_cast<const google::protobuf::Message*>(last_d_state);
		assert(d_msg);

	// XXX - redesign it
	for(int i=0; i<plots.size(); i++){
		if(ptr != plots[i])
			continue;
		if(plot_special_flags[i])
			ptr->saveToCsv(file, last_special, NULL, last_special_time);
		else
			ptr->saveToCsv(file, msg, d_msg, last_time);
	}
}

void ChartAnalyzer::on_add_clicked(){
	const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(last_state);
	assert(msg);
	(new ChartAddDialog(this, msg))->show_all();
}

void ChartAnalyzer::on_add_special_clicked(){
	assert(special_msg);
	(new ChartAddDialog(this, special_msg))->show_all();
}

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

// TODO: yrange param is too much %)
Gtk::Widget* ChartAnalyzer::addChart(const google::protobuf::Message* msg, std::vector<std::string> vars, std::string x_axis_var, bool polar, double yrange){
	std::ostringstream full_title;

	Gtk::Socket* socket = Gtk::manage(new Gtk::Socket());

	Gnuplot* p = new Gnuplot(socket->get_id());
	if(yrange > 0){
			p->setYRange(0, yrange);
	}

	p->setPolar(polar);
//	p->addExpression("2* ($particles.a'-$E)");

	for(int i=0; i<vars.size(); i++){
		p->addVar(vars[i]);
		full_title << vars[i] << ' ';
	}// for
	if(!x_axis_var.empty()){
		p->setXAxisVar(x_axis_var);
		full_title << "| " << x_axis_var << " ";
	}

	// set dots for arrays
	if(vars[0].find('.')!=std::string::npos)
		p->setStyle(Gnuplot::STYLE_POINTS);


	p->setTitle(trim(full_title.str()));
	plots.push_back(p);	// XXX: not very copyable - but with no copies it will work...
	plot_special_flags.push_back(msg==this->special_msg);

	// add widget for it //
	Gtk::Button* del = new Gtk::Button("del");
	Gtk::Button* writeback = new Gtk::Button("auto");
	Gtk::Button* restore = new Gtk::Button("fixed");
	Gtk::Button* save = new Gtk::Button("save");
	Gtk::Label* l = new Gtk::Label(full_title.str());

	Gtk::HBox* hbox = new Gtk::HBox();
	hbox->pack_end(*Gtk::manage(del), false, false);
	hbox->pack_end(*Gtk::manage(writeback), false, false);
	hbox->pack_end(*Gtk::manage(restore), false, false);
	hbox->pack_end(*Gtk::manage(save), false, false);
	hbox->pack_start(*Gtk::manage(l), true, true);

	del->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ChartAnalyzer::on_del_chart_clicked), (Gtk::Widget*)hbox, p));
	writeback->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ChartAnalyzer::on_writeback_clicked), p));
	restore->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ChartAnalyzer::on_restore_clicked), p));
	save->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ChartAnalyzer::on_save_clicked), p));
	vbox.pack_start(*Gtk::manage(hbox), false, false);
	hbox->show_all();

	p->processState(msg);

	return socket;
}

void ChartAnalyzer::on_del_chart_clicked(Gtk::Widget* w, const Gnuplot* ptr){
	assert(ptr);
	for(int i=0; i<plots.size(); i++){
		if(plots[i] == ptr){
			delete plots[i];
			plots.erase(plots.begin()+i);
			vbox.remove(*w);
			return;
		}// if
	}// for
}

void ChartAnalyzer::on_writeback_clicked(Gnuplot* ptr){
	ptr->writeback();
	//ptr->processState(dynamic_cast<const Message*>(last_state), dynamic_cast<const Message*>(last_d_state), last_time);
}

void ChartAnalyzer::on_restore_clicked(Gnuplot* ptr){
	ptr->restore();
}

