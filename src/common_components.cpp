/*
 * common_components.cpp
 *
 *  Created on: Dec 26, 2016
 *      Author: dimalit
 */

#include "common_components.h"

#include <gtkmm/table.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/socket.h>

#include <sstream>
#include <iostream>

#define UI_FILE "charts_add.glade"

EXPetscSolverConfig::EXPetscSolverConfig(){
	set_atol(1e-6);
	set_rtol(1e-6);
	set_init_step(0.01);
	set_n_cores(1);
}

// XXX: whis should read like E4...if we use more than 1 specialization?
const char* EXPetscSolver::ts_path = " ../ts4/Debug/ts4";

EXPetscSolver::EXPetscSolver(const EXPetscSolverConfig* scfg, const OdeConfig* pcfg, const OdeState* init_state){
	time_passed = 0;
	steps_passed = 0;

	pconfig = pcfg->clone();
	sconfig = new EXPetscSolverConfig(*scfg);
	state = init_state->clone();
	// TODO Here ws just new State. Make virtual New function?
	d_state = init_state->clone();
}

EXPetscSolver::~EXPetscSolver(){
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
void EXPetscSolver::run(int steps, double time, bool use_step){
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

bool EXPetscSolver::step(){
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

bool EXPetscSolver::read_results(){
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


	::google::protobuf::Message* state = dynamic_cast<::google::protobuf::Message*>(this->state);
	::google::protobuf::Message* d_state = dynamic_cast<::google::protobuf::Message*>(this->d_state);

	state->CopyFrom(sol.state());
	d_state->CopyFrom(sol.d_state());
	return true;
}

void EXPetscSolver::finish(){
	fputc('f', wf);
	fflush(wf);
}

AbstractConfigWidget::AbstractConfigWidget(Message *msg){
	this->data = msg;

	grid.set_column_spacing(4);
	grid.set_row_spacing(4);
	grid.set_margin_left(4);
	grid.set_margin_top(4);
	grid.set_margin_right(4);
	grid.set_margin_bottom(4);
	this->add(grid);

	button_apply.set_label("Apply");
	button_apply.set_hexpand(true);
	button_apply.signal_clicked().connect(sigc::mem_fun(*this, &AbstractConfigWidget::on_apply_cb));

	construct_ui();
	config_to_widget();
}

void AbstractConfigWidget::construct_ui(){

	// 1 clear
	std::vector<Widget*> v = grid.get_children();
	for(auto it=v.begin(); it!=v.end(); ++it){
		Gtk::Widget* w = *it;
		grid.remove(*w);
		if(w != &button_apply)
			delete w;
	}// for

	// 2 add
	grid.attach(button_apply, 0, 0, 2, 1);

	if(!data)
		return;

	const Descriptor* d = data->GetDescriptor();

	for(int i=0; i<d->field_count(); i++){
		const FieldDescriptor* fd = d->field(d->field_count()-i-1);
		const string& fname = fd->name();

		Gtk::Label *label = Gtk::manage(new Gtk::Label);
		label->set_text(fname);

		Gtk::Entry *entry = Gtk::manage(new Gtk::Entry);
		entry->set_hexpand(true);

		grid.insert_row(0);
		grid.attach(*label, 0, 0, 1, 1);
		grid.attach(*entry, 1, 0, 1, 1);

		entry_map[fname] = entry;
		entry->signal_changed().connect(sigc::mem_fun(*this, &AbstractConfigWidget::edit_anything_cb));
	}// for fields
}

void AbstractConfigWidget::setData(Message* msg){
	data = msg;
	construct_ui();
	config_to_widget();
}

const Message* AbstractConfigWidget::getData() const{
	widget_to_config();
	return data;
}

void AbstractConfigWidget::widget_to_config() const{
	if(data == NULL)
		return;

	const Descriptor* desc = data->GetDescriptor();
	const Reflection* refl = data->GetReflection();

	for(auto i=entry_map.begin(); i!=entry_map.end(); ++i){
		const string& var = i->first;
		Gtk::Entry* entry = i->second;

		string val = entry->get_text();

		const FieldDescriptor* fd = desc->FindFieldByName(var);
		FieldDescriptor::Type type = fd->type();

		switch(type){
			case FieldDescriptor::Type::TYPE_DOUBLE:
				refl->SetDouble(data, fd, atof(val.c_str()));
				break;
			case FieldDescriptor::Type::TYPE_FLOAT:
				refl->SetFloat(data, fd, (float)atof(val.c_str()));
				break;
			case FieldDescriptor::Type::TYPE_INT32:
				refl->SetInt32(data, fd, atoi(val.c_str()));
				break;
			case FieldDescriptor::Type::TYPE_UINT32:
				refl->SetUInt32(data, fd, atoi(val.c_str()));
				break;
			case FieldDescriptor::Type::TYPE_STRING:
				refl->SetString(data, fd, val);
				break;
			default:
				assert(false);
		}

	}// for

	button_apply.set_sensitive(false);
}
void AbstractConfigWidget::config_to_widget() const {
	if(data == NULL)
		return;

	const Descriptor* desc = data->GetDescriptor();
	const Reflection* refl = data->GetReflection();

	for(auto i=entry_map.begin(); i!=entry_map.end(); ++i){
		const string& var = i->first;
		Gtk::Entry* entry = i->second;

		std::ostringstream res;

		const FieldDescriptor* fd = desc->FindFieldByName(var);
		FieldDescriptor::Type type = fd->type();

		switch(type){
			case FieldDescriptor::Type::TYPE_DOUBLE:
				res << refl->GetDouble(*data, fd);
				break;
			case FieldDescriptor::Type::TYPE_FLOAT:
				res << refl->GetFloat(*data, fd);
				break;
			case FieldDescriptor::Type::TYPE_INT32:
				res << refl->GetInt32(*data, fd);
				break;
			case FieldDescriptor::Type::TYPE_UINT32:
				res << refl->GetUInt32(*data, fd);
				break;
			case FieldDescriptor::Type::TYPE_STRING:
				res << refl->GetString(*data, fd);
				break;
			default:
				assert(false);
		}

		entry->set_text(res.str());
	}// for

	button_apply.set_sensitive(false);
}

void AbstractConfigWidget::edit_anything_cb(){
	button_apply.set_sensitive(true);
}

void AbstractConfigWidget::on_apply_cb(){
	widget_to_config();
	m_signal_changed.emit();
}

///////////////////////////////////////////////////////////////////////////////
EXConfigWidget::EXConfigWidget(const OdeConfig* cfg){
	this->add(cfg_widget);

	if(!cfg)
		this->config = NULL;
	else
		this->config = cfg->clone();

	cfg_widget.setData(dynamic_cast<Message*>(config));
	cfg_widget.signal_changed().connect(sigc::mem_fun(*this, &EXConfigWidget::on_changed));
}

void EXConfigWidget::on_changed(){
	m_signal_changed.emit();
}

void EXConfigWidget::loadConfig(const OdeConfig* cfg){
	if(!cfg)
		this->config = NULL;
	else
		this->config = cfg->clone();

	Message* msg = dynamic_cast<Message*>(config);
		assert(msg);
	cfg_widget.setData(msg);
}

const OdeConfig* EXConfigWidget::getConfig() const{
	return this->config;
}

///////////////////////////////////////////////////////////////////////////////

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

		parent->addChart(msg, vars, x_axis_var, NULL, polar);

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
void ChartAnalyzer::addChart(const google::protobuf::Message* msg, std::vector<std::string> vars, std::string x_axis_var, Gtk::Container* parent, bool polar, double yrange){
	std::ostringstream full_title;

	int socket_id = 0;

	if(parent){
		Gtk::Socket* socket = Gtk::manage(new Gtk::Socket());
		parent->add(*socket);
		parent->show_all();
		socket_id = socket->get_id();
		std::cerr << "Socket: " << std::hex << socket_id << "\n";
	}
	Gnuplot* p = new Gnuplot(socket_id);
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
