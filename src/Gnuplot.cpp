/*
 * Gnuplot.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: dimalit
 */

#include "Gnuplot.h"
#include <vector>
#include <algorithm>

#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cmath>
#include "rpc.h"

using namespace google::protobuf;

std::map<std::string, std::string> Gnuplot::title_translation_map;

std::string extract_child_part(const std::string& s, std::string* parent=NULL){
	int pos = s.find('.');
	if(pos == std::string::npos){
		return s;			// ignore prefix for vars without it
	}
	else{
		if(parent)
			*parent = s.substr(0, pos);
		return s.substr(pos+1);
	}
}

std::string extract_vars(const std::string& expr, std::vector<std::string>& columns, int varno){
	// replace var names with numbers starting at varno
	char* dup = strdup(expr.c_str());
	char var[45];
	char* p = dup;
	while(p=strchr(p, '$')){
		sscanf(p, "%[$a-zA-Z0-9_.']", var);
		// remember. duplicates are OK
		columns.push_back(extract_child_part(var+1));	// exclude $ sign
		// remove from p
		int varlen = strlen(var)-1;
		char* beg = strstr(p, var+1);
		//*beg = '0'+varno;
		sprintf(beg, "%d", varno);
		int num_len = strlen(beg);
		for(char* cur=beg+num_len; *(cur+varlen-num_len-1); cur++){
			*cur = *(cur+varlen-num_len);
		}// for
		p = beg+1;
		varno++;
	}

	std::string res(dup);
	free(dup);
	return res;
}

Gnuplot::Gnuplot(int x_win_id) {
	this->x_win_id = x_win_id;

	if(x_win_id==0){
		width = 400;
		height = 400;
	}
	else{
		width = 600;
		height = 200;
	}

	title = "";
	x_axis = "";			// time/index
	style = STYLE_LINES;
	polar = 0;
	parametric = false;

	int rf, wf;
	rpc_call("gnuplot", &rf, &wf);
	to_gnuplot = fdopen(wf, "wb");

	if(!to_gnuplot){
		perror(NULL);
		assert(false);
	}

	fprintf(to_gnuplot, "set encoding utf8\n");
	fprintf(to_gnuplot, "symbol(z) = \"•✷+△♠□♣♥♦\"[int(z):int(z)]\n");
	fprintf(to_gnuplot, "modf(x,m) = (x/m-floor(x/m))*m\n");
	fprintf(to_gnuplot, "set view 0,90\n");
	fflush(to_gnuplot);

	update_view();
}

void Gnuplot::processState(const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	printPlotCommand(to_gnuplot, msg, d_msg, time);
}

std::string get_val(const Message* msg, const Message* d_msg, const std::string& var_name){
	bool derivative = var_name[var_name.length()-1]=='\'';
	string my_name = var_name;

	if(derivative){
		my_name = var_name.substr(0, var_name.length()-1);
		msg = d_msg;
	}

	const Descriptor* desc = msg->GetDescriptor();
	const Reflection* refl = msg->GetReflection();
	const FieldDescriptor* fd = desc->FindFieldByName(my_name);
	assert(fd);
	double val = refl->GetDouble(*msg, fd);

	// HACK for model e4
	if(var_name == "psi"){
		const FieldDescriptor* fd2 = desc->FindFieldByName("z");
		assert(fd2);
		double val2 = refl->GetDouble(*msg, fd2);
		val -= 2*M_PI*val2;
	}

	return ((std::ostringstream&)(std::ostringstream() << val)).str();
}

std::string get_vals(const Message* msg, const Message* d_msg, std::vector<std::string> vars, const Message* parent_msg=NULL, const Message* parent_d_msg=NULL){
	const Descriptor* desc = msg->GetDescriptor();
	const Reflection* refl = msg->GetReflection();
	const Descriptor* d_desc = d_msg ? msg->GetDescriptor() : NULL;
	const Reflection* d_refl = d_msg ? msg->GetReflection() : NULL;

	const Descriptor* p_desc = parent_msg ? parent_msg->GetDescriptor() : NULL;
	const Reflection* p_refl = parent_msg ? parent_msg->GetReflection() : NULL;
	const Descriptor* p_d_desc = parent_d_msg ? parent_msg->GetDescriptor() : NULL;
	const Reflection* p_d_refl = parent_d_msg ? parent_msg->GetReflection() : NULL;

	std::ostringstream res;
	for(int i=0; i<vars.size(); i++){
		std::string var_name = vars[i];
		bool derivative = var_name[var_name.length()-1]=='\'';
		string my_name = var_name;

		if(derivative)
			my_name = var_name.substr(0, var_name.length()-1);
		const FieldDescriptor* fd = derivative ? d_desc->FindFieldByName(my_name) : desc->FindFieldByName(my_name);
		// XXX: if var name in child collides with the one in parent - will get value from child instead of parent
		if(!fd){
			fd = derivative ? p_d_desc->FindFieldByName(my_name) : p_desc->FindFieldByName(my_name);
			assert(fd);
			res << (derivative ? p_d_refl->GetDouble(*parent_d_msg, fd) : p_refl->GetDouble(*parent_msg, fd)) << "\t";
		}
		else
			res << (derivative ? d_refl->GetDouble(*msg, fd) : refl->GetDouble(*msg, fd)) << "\t";
	}// for
	return res.str();
}

void Gnuplot::print_numbers(const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time, serie& s, std::ostream& out, bool add_to_cache){
	const Descriptor* d_desc = d_msg ? d_msg->GetDescriptor() : NULL;
	const Reflection* d_refl = d_msg ? d_msg->GetReflection() : NULL;

	bool need_series_wrap = false;//!!getXAxisTime() && !polar && this->x_axis.find("ksi") != std::string::npos;
	bool is_particles = s.var_name.find("particles") != std::string::npos;
	bool need_coloring = false;//s.var_name=="particles.a" && d_msg;

	// simple field
	if(s.var_name.find('.') == std::string::npos){
		// if need time
		if(getXAxisTime() || getParametric()){
			ostringstream stime; stime << time;
			if(add_to_cache){
				std::string yval = s.is_expression ? get_vals(msg, d_msg, s.columns) : get_val(msg, d_msg, s.var_name);
				std::string xval = getParametric() ? get_val(msg, d_msg, this->x_axis) : stime.str();
				s.data_cache.push_back(make_pair(xval, yval));
			}

			for(int i=0; i<s.data_cache.size(); i++){
				out << s.data_cache[i].first << " " << s.data_cache[i].second << "\n";
			}// for
		}// if need time
		else{
			std::string yval = s.is_expression      ? get_vals(msg, d_msg, s.columns) : get_val(msg, d_msg, s.var_name);
			std::string xval = x_columns.size() > 0 ? get_vals(msg, d_msg, x_columns) : get_val(msg, d_msg, this->x_axis);
			out << xval << " " << yval << "\n";
		}// not need time
	}// if simple field
	else{	// repeated field
		std::string f1, f2;
		if(!s.is_expression){
			f1 = s.var_name.substr(0, s.var_name.find('.'));	// parent
				assert(f1.size());
			f2 = s.var_name.substr(s.var_name.find('.')+1);		// child
				assert(f2.size());
		}
		else{
			f1 = s.prefix;
			assert(!f1.empty());
		}

		const Descriptor* desc = msg->GetDescriptor();
		const Reflection* refl = msg->GetReflection();

		const FieldDescriptor* fd1 = desc->FindFieldByName(f1);
		const FieldDescriptor* d_fd1 = d_desc ? d_desc->FindFieldByName(f1) : NULL;
			assert(!d_desc || fd1);
		int n = refl->FieldSize(*msg, fd1);
			assert(!d_fd1 || refl->FieldSize(*msg, fd1) == d_refl->FieldSize(*msg, d_fd1));

		for(int i=0; i<n; i++){
			string y;
			if(!s.is_expression)
				y = get_val(&refl->GetRepeatedMessage(*msg, fd1, i), d_refl ? &d_refl->GetRepeatedMessage(*d_msg, d_fd1, i) : NULL, f2);
			else
				y = get_vals(&refl->GetRepeatedMessage(*msg, fd1, i), d_refl ? &d_refl->GetRepeatedMessage(*d_msg, d_fd1, i) : NULL, s.columns, msg, d_msg);
			std::ostringstream oss;
			oss << i;
			std::string x = oss.str();

			// set x to value of specific x variable if needed
			if(!getXAxisTime()){
				std::string xname = this->x_axis;
				// remove all before dot
				if(this->x_axis.find('.') != std::string::npos)
					xname = x_axis.substr(x_axis.find('.')+1);

				x = x_columns.size() > 0 ?
						get_vals(&refl->GetRepeatedMessage(*msg, fd1, i), d_refl ? &d_refl->GetRepeatedMessage(*d_msg, fd1, i) : NULL, x_columns) :
						get_val(&refl->GetRepeatedMessage(*msg, fd1, i), d_refl ? &d_refl->GetRepeatedMessage(*d_msg, fd1, i) : NULL, xname);
//				if(xname=="ksi" && polar)
//					x *= 2.0*M_PI;

				// draw or not?
				// this was for limiting to 100 particles
//				double xc = x - floor(x/2.0/M_PI)*2.0*M_PI;
//				int x_pos = xc/2.0/M_PI*100;
//				if(x_pos >= 100)
//					x_pos = 99;
//				if(is_particles && !polar && x_bits[x_pos])
//					continue;
//
//				x_bits[x_pos] = true;
			}// if

			// start new series if needed
//			if(need_series_wrap && prev_x>-99 && abs(x-prev_x)>0.5){
//				super_buffer << "e\n";
//				plot_command << ", '-' with " << style << " title '" << title <<"'";
//			}

//			if(is_particles){
//				x += atof(get_val(&refl->GetRepeatedMessage(*msg, fd1, i), NULL, "x").c_str());
//				double yy;
//				sscanf(y.c_str(), "%lf", &yy);
//				yy += atof(get_val(&refl->GetRepeatedMessage(*msg, fd1, i), NULL, "y").c_str());
//				stringstream ss;
//				ss << yy;
//				y = ss.str();
//			}

			if(polar)
				out << x << " " << y;
			else
				out << x << " " << y;

			if(need_coloring){
				const Message& d_m2 = d_refl->GetRepeatedMessage(*d_msg, d_fd1, i);
				double dy = atof(get_val(NULL, &d_m2, f2+"'").c_str());
				if(dy>0)
					out << " 7";
				else
					out << " 1";
			}

			if(is_particles){
				double z = atof(get_val(&refl->GetRepeatedMessage(*msg, fd1, i), NULL, "z").c_str());
				//z = fmod(z, 1.0);
				//z = cos(2*M_PI*z);
				out << " " << z;
			}


			out << "\n";

		}// for points
	}// if repeated field
}

void Gnuplot::printPlotCommand(FILE* fp, const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	assert(msg);

	if(x_axis[x_axis.length()-1]=='\'' && !d_msg)
		return;

//	msg->PrintDebugString();
//	d_msg->PrintDebugString();

	std::ostringstream plot_command;			// command
	std::ostringstream super_buffer;			// data
		super_buffer.precision(10);

	plot_command << "set size square\n";

	if(polar){
		plot_command << "set polar\n";
		//plot_command << draw_bells(msg) << "\n";
	}
	if(parametric)
		plot_command << "set parametric\n";

	//plot_command << "splot ";

	for(int i=0; i<series.size(); i++){
		serie& s = series[i];

		bool need_coloring = false;//s.var_name=="particles.a" && d_msg;
		bool need_boxes = s.var_name=="hist.y";
		bool is_particles = s.var_name.find("particles") != std::string::npos;

		if(i==0 && is_particles && !polar)
			plot_command << "splot ";
		else if(i==0)
			plot_command << "plot ";

		if(s.var_name[s.var_name.length()-1]=='\'' && !d_msg)
			continue;

		if(i!=0)
			plot_command << ", ";

		std::string palette = is_particles ? "palette " : "";
		std::string style = this->style == STYLE_LINES ? "lines" : "points " + palette + "ps 0.5";

		if(need_boxes)
			style = "boxes";

		if(need_coloring){
			style += " lc variable";
		}
		string title = s.var_name;
		if(title_translation_map.count(title))
			title = title_translation_map[title];
//		if(polar)
//			plot_command << "0.5, ";

		plot_command << "'-' ";
		if(x_columns.size()==0)
			plot_command << "using ($1):(" << s.expression << ")";
		else
			plot_command << "using ("<< x_axis <<"):(" << s.expression << ")";
		if(is_particles){
			int x_columns_size = x_columns.size() == 0 ? 1 : x_columns.size();
			int y_columns_size = s.columns.size() == 0 ? 1 : s.columns.size();
			plot_command << ":($" << x_columns_size+y_columns_size+1 << ")";
		}
		plot_command << " ";

		plot_command << "with " << style << " title \"" << title <<"\"";

		print_numbers(msg, d_msg, time, s, super_buffer);

		super_buffer << "e\n";
	}// for series

	// print all!
	plot_command << "\n";

//	std::cerr << plot_command.str() << std::endl;
//	std::cerr << super_buffer.str() << std::endl;

	fprintf(fp, plot_command.str().c_str());
	fprintf(fp, super_buffer.str().c_str());
	fflush(fp);

//	printf(plot_command.str().c_str());
//	printf(super_buffer.str().c_str());
}

void Gnuplot::processToFile(const std::string& file, const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	fprintf(to_gnuplot, "set terminal png\n");
	fprintf(to_gnuplot, "set output \"%s\"\n", file.c_str());
	processState(msg, d_msg, time);
	if(x_win_id)
		fprintf(to_gnuplot, "set terminal x11 window \"%x\"\n", x_win_id);
	else
		fprintf(to_gnuplot, "set terminal x11 window\n");
	fflush(to_gnuplot);

}

void Gnuplot::saveToCsv(const std::string& file, const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	FILE* fp = fopen(file.c_str(), "wb");
	assert(fp);		// TODO: exception
	fprintf(fp, "set label \"t = %.2lf\" at graph 0.02, graph 0.95\n", time);
	printPlotCommand(fp, msg, d_msg, time);
	fclose(fp);
}

void Gnuplot::saveSerie(int ser_no, const std::string& file, const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	assert(ser_no < this->series.size());
	std::ofstream out(file);
	serie& s = this->series[ser_no];
	this->print_numbers(msg, d_msg, time, s, out, false);			// don't add to cache
	out.close();
}

void Gnuplot::setXAxisVar(std::string var){
	style = STYLE_POINTS;
	x_axis = var;
	if(var.find('$') != std::string::npos){
		int s_pos = var.find('$');
		x_axis = extract_vars(var, this->x_columns, 1);
		int dot_pos = var.find('.');
		if(dot_pos != std::string::npos)
			this->x_prefix = var.substr(s_pos+1, dot_pos-s_pos-1);
	}
}

void Gnuplot::addVar(std::string var){
	serie s;
	s.var_name = var;
	int x_columns_size = this->x_columns.size();
	if(x_columns_size==0)
		x_columns_size = 1;			// for possible t
	// for expressions
	if(var.find('$') != std::string::npos){
		int s_pos = var.find('$');
		s.is_expression = true;
		s.expression = extract_vars(var, s.columns, x_columns_size+1);
		int dot_pos = var.find('.');
		if(dot_pos != std::string::npos)
			s.prefix = var.substr(s_pos+1, dot_pos-s_pos-1);
	}// else
	series.push_back(s);
}

void Gnuplot::eraseVar(int idx){
	assert(idx >= 0 && idx <series.size());
	series.erase(series.begin()+idx);
}

void Gnuplot::writeback() {
	fprintf(to_gnuplot, "set xrange [*:*] writeback\n");
	fprintf(to_gnuplot, "set yrange [*:*] writeback\n");
	fflush(to_gnuplot);
}
void Gnuplot::restore() {
	fprintf(to_gnuplot, "set xrange restore\n");
	fprintf(to_gnuplot, "set yrange restore\n");
	fflush(to_gnuplot);
}

void Gnuplot::setXRange(double from, double to){
	if(to != std::numeric_limits<double>::infinity())
		fprintf(to_gnuplot, "set xrange [%lf:%lf]\n", from, to);
	else
		fprintf(to_gnuplot, "set xrange [%lf:*]\n", from);
	fflush(to_gnuplot);
}
void Gnuplot::setYRange(double from, double to){
	if(to != std::numeric_limits<double>::infinity())
		fprintf(to_gnuplot, "set yrange [%lf:%lf]\n", from, to);
	else
		fprintf(to_gnuplot, "set yrange [%lf:*]\n", from);
	fflush(to_gnuplot);
}

Gnuplot::~Gnuplot() {
	fprintf(to_gnuplot, "exit\n");
	fflush(to_gnuplot);
	fclose(to_gnuplot);
}

void Gnuplot::update_view(){
	stringstream win;
	if(x_win_id)
		win << "window \"0x" << std::hex << x_win_id << "\"";
	fprintf(to_gnuplot, "set terminal x11 size %d, %d title \"%s\" %s noraise\n", width, height, title.c_str(), win.str().c_str());
	//fprintf(to_gnuplot, "set xrange [*:*] writeback\n");
//	fprintf(to_gnuplot, "set xrange [-0.25:-0.05] writeback\n");
//	fprintf(to_gnuplot, "set yrange [0:0.01] writeback\n");
	fflush(to_gnuplot);
}

std::string Gnuplot::draw_bells(const google::protobuf::Message* msg){
	const Descriptor* desc = msg->GetDescriptor();
	const Reflection* refl = msg->GetReflection();

	std::stringstream plot_command;

	const google::protobuf::FieldDescriptor *a0_fd = desc->FindFieldByName("a0");
	double a0 = refl->GetDouble(*msg,a0_fd);
//	plot_command << "set object circle at 0,0 size "<< a0 <<"\n";

	const google::protobuf::FieldDescriptor *phi_fd = desc->FindFieldByName("phi");
	double phi = refl->GetDouble(*msg,phi_fd);

	const google::protobuf::FieldDescriptor *e_fd = desc->FindFieldByName("E");
	double e = refl->GetDouble(*msg,e_fd);

	plot_command << "set arrow 1 from 0,0 to " << 10*e*cos(phi) << "," << 10*e*sin(phi) << " linecolor 1\n";

	// arrow for Aj
	const FieldDescriptor* fd1 = desc->FindFieldByName("particles");
	int n = refl->FieldSize(*msg, fd1);

	double x=0, y=0;			// particles
	double nlx = 0, nly = 0;	// non-linearity
	for(int i=0; i<n; i++){
		string sa = get_val(&refl->GetRepeatedMessage(*msg, fd1, i), NULL, "a");
		string spsi = get_val(&refl->GetRepeatedMessage(*msg, fd1, i), NULL, "psi");
		double a, psi;
		sscanf(sa.c_str(), "%lf", &a);
		sscanf(spsi.c_str(), "%lf", &psi);

		x += a*cos(psi);
		y += a*sin(psi);

		double a2 = a*a-a0*a0;
		nlx += -a2*a*sin(psi);
		nly +=  a2*a*cos(psi);
	}
	double alpha = 0.1;
	nlx *= alpha/n;
	nly *= alpha/n;

	// a
	plot_command << "set arrow 2 from 0,0 to " << 10*x/n << "," << 10*y/n << " linecolor 2\n";
	//-theta*E
	double theta = 1;
	plot_command << "set arrow 3 from " << 10*x/n << "," << 10*y/n << " rto " << -theta*10*e*cos(phi) << "," << -theta*10*e*sin(phi) << " linecolor 2\n";

	//-e
	plot_command << "set arrow 4 from 0,0 to " << -10*e*cos(phi) << "," << -10*e*sin(phi) << " linecolor 3\n";
	//nl
	plot_command << "set arrow 5 from " << -10*e*cos(phi) << "," << -10*e*sin(phi) << " rto " << nlx*10 << ", " << nly*10 << " linecolor 3\n";

	return plot_command.str();
}
