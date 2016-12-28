/*
 * common_components.cpp
 *
 *  Created on: Dec 26, 2016
 *      Author: dimalit
 */

#include "common_components.h"

#include <sstream>
#include <iostream>

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


EXPetscSolverConfigWidget::EXPetscSolverConfigWidget(const EXPetscSolverConfig* config){
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

const OdeSolverConfig* EXPetscSolverConfigWidget::getConfig(){
	widget_to_config();
	return config;
}

void EXPetscSolverConfigWidget::loadConfig(const OdeSolverConfig* config){
	const EXPetscSolverConfig* econfig = dynamic_cast<const EXPetscSolverConfig*>(config);
		assert(econfig);
	delete this->config;
	this->config = new EXPetscSolverConfig(*econfig);
	config_to_widget();
}

void EXPetscSolverConfigWidget::widget_to_config(){
	config->set_init_step(atof(entry_step->get_text().c_str()));
	config->set_atol(atof(entry_atol->get_text().c_str()));
	config->set_rtol(atof(entry_rtol->get_text().c_str()));
	config->set_n_cores(adj_n_cores->get_value());
}

void EXPetscSolverConfigWidget::config_to_widget(){
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

///////////////////////////////////////////////////////////////////////////////

MessageChart::MessageChart(const std::vector<std::string>& vars, Gtk::Container* parent){
	last_time = 0;
	last_msg = NULL;
	last_d_msg = NULL;

	this->vars = vars;

	int socket_id = 0;
	if(parent){
		Gtk::Socket* socket = Gtk::manage(new Gtk::Socket());
		parent->add(*socket);
		parent->show_all();
		socket_id = socket->get_id();
		std::cerr << "Socket: " << std::hex << socket_id << "\n";
	}
	gnuplot = new Gnuplot(socket_id);

	for(int i=0; i<vars.size(); i++)
		gnuplot->addVar(vars[i]);

	// set dots for arrays
	if(vars[0].find('.')!=std::string::npos)
		gnuplot->setStyle(Gnuplot::STYLE_POINTS);

	// add widgets //
	Gtk::Button* del = new Gtk::Button("del");
	Gtk::Button* writeback = new Gtk::Button("auto");
	Gtk::Button* restore = new Gtk::Button("fixed");
	Gtk::Button* save = new Gtk::Button("save");

	Gtk::HBox* hbox = new Gtk::HBox();
	hbox->pack_end(*Gtk::manage(del), false, false);
	hbox->pack_end(*Gtk::manage(writeback), false, false);
	hbox->pack_end(*Gtk::manage(restore), false, false);
	hbox->pack_end(*Gtk::manage(save), false, false);
	hbox->pack_start(label, true, true);

	del->signal_clicked().connect(sigc::mem_fun(*this, &MessageChart::on_del_chart_clicked));
	writeback->signal_clicked().connect(sigc::mem_fun(*this, &MessageChart::on_writeback_clicked));
	restore->signal_clicked().connect(sigc::mem_fun(*this, &MessageChart::on_restore_clicked));
	save->signal_clicked().connect(sigc::mem_fun(*this, &MessageChart::on_save_clicked));
	this->add(*Gtk::manage(hbox));

	update_title();
}

void MessageChart::update_title(){
	std::ostringstream full_title;

	for(int i=0; i<vars.size(); i++){
		full_title << vars[i] << ' ';
	}// for
	if(!gnuplot->getXAxisVar().empty()){
		full_title << "| " << gnuplot->getXAxisVar() << " ";
	}

	gnuplot->setTitle(trim(full_title.str()));
	label.set_text(full_title.str());
}

void MessageChart::processMessage(const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	assert(msg);

	gnuplot->processState(msg, d_msg, time);

	delete last_msg;
	last_msg = msg->New();
	last_msg->CopyFrom(*msg);
	if(d_msg){
		delete last_d_msg;
		last_d_msg = d_msg->New();
		last_d_msg->CopyFrom(*d_msg);
	}
}

void MessageChart::on_save_clicked(){
	assert(last_msg);
	std::string file = gnuplot->getTitle()+".csv";
	gnuplot->saveToCsv(file, last_msg, last_d_msg, last_time);
}

void MessageChart::on_del_chart_clicked(){
	this->signal_closed.emit();
}

void MessageChart::on_writeback_clicked(){
	gnuplot->writeback();
}

void MessageChart::on_restore_clicked(){
	gnuplot->restore();
}

///////////////////////////////////////////////////////////////////////////////

EXChartAnalyzer::EXChartAnalyzer(const OdeConfig* config) {
	this->config = config->clone();

	states_count = 0;

	btn_add.set_label("Add chart");
	vbox.pack_start(btn_add);
	btn_add.signal_clicked().connect(sigc::mem_fun(*this, &EXChartAnalyzer::on_add_clicked));

	btn_reset.set_label("Reset");
	vbox.pack_start(btn_reset);
	btn_reset.signal_clicked().connect(sigc::mem_fun(*this, &EXChartAnalyzer::reset));

	this->add(vbox);
}

EXChartAnalyzer::~EXChartAnalyzer() {
	for(int i=0; i<charts.size(); i++)
		delete charts[i];
	delete config;
}

int EXChartAnalyzer::getStatesCount(){
	return states_count;
}

void EXChartAnalyzer::reset(){
	states_count = 0;
	for(int i=0; i<charts.size(); i++)
		charts[i]->reset();
}

void EXChartAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(state);
		assert(msg);
	const google::protobuf::Message* d_msg = dynamic_cast<const google::protobuf::Message*>(d_state);
		assert(d_msg);

	for(int i=0; i<charts.size(); i++)
		charts[i]->processMessage(msg, d_msg, time);
}

void EXChartAnalyzer::addChart(MessageChart* chart){
	charts.push_back(chart);
	vbox.pack_end(*chart, false, false, 20);
}

void EXChartAnalyzer::on_add_clicked(){
	const google::protobuf::Message* msg = new_state();
	ChartAddDialog* dialog = new ChartAddDialog(msg, true);
	dialog->signal_cancel.connect(
			sigc::bind(
					sigc::mem_fun(*this, &EXChartAnalyzer::on_dialog_cancel),
					dialog
			)
	);
	dialog->signal_ok.connect(
			sigc::bind(
					sigc::mem_fun(*this, &EXChartAnalyzer::on_dialog_add_ok),
					dialog
			)
	);

	dialog->show_all();
}

void EXChartAnalyzer::on_dialog_add_ok(ChartAddDialog* dialog){
	MessageChart* chart = new MessageChart(dialog->vars, NULL);
	addChart(chart);
	delete dialog;
}

void EXChartAnalyzer::on_dialog_cancel(ChartAddDialog* dialog){
	delete dialog;
}

void EXChartAnalyzer::on_del_chart_clicked(const MessageChart* chart){
	assert(chart);
	for(int i=0; i<charts.size(); i++){
		if(charts[i] == chart){
			delete charts[i];
			charts.erase(charts.begin()+i);
			return;
		}// if
	}// for
}
