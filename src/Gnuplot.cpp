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
#include <iostream>
#include <cstdio>
#include "rpc.h"

using namespace google::protobuf;

std::map<std::string, std::string> Gnuplot::title_translation_map;

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

	int rf, wf;
	rpc_call("gnuplot", &rf, &wf);
	to_gnuplot = fdopen(wf, "wb");

	if(!to_gnuplot){
		perror(NULL);
		assert(false);
	}

	fprintf(to_gnuplot, "set encoding utf8\n");
	fprintf(to_gnuplot, "symbol(z) = \"•✷+△♠□♣♥♦\"[int(z):int(z)]\n");
	fflush(to_gnuplot);

	update_view();
}

void Gnuplot::processState(const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	printPlotCommand(to_gnuplot, msg, d_msg, time);
}

double get_val(const Message* msg, const Message* d_msg, const std::string& var_name){
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
	return refl->GetDouble(*msg, fd);
}

void Gnuplot::printPlotCommand(FILE* fp, const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	assert(msg);

	const Descriptor* desc = msg->GetDescriptor();
	const Reflection* refl = msg->GetReflection();
	const Descriptor* d_desc = d_msg ? d_msg->GetDescriptor() : NULL;
	const Reflection* d_refl = d_msg ? d_msg->GetReflection() : NULL;

	if(x_axis[x_axis.length()-1]=='\'' && !d_msg)
		return;

//	msg->PrintDebugString();
//	d_msg->PrintDebugString();

	std::ostringstream plot_command;			// command
	std::ostringstream super_buffer;			// data
		super_buffer.precision(10);

	if(polar){
		plot_command << "set polar\n";
//		plot_command << draw_bells(msg, desc, refl);
	}
	plot_command << "plot ";

	for(int i=0; i<series.size(); i++){
		serie& s = series[i];

		bool need_series_wrap = false;//!!getXAxisTime() && !polar && this->x_axis.find("ksi") != std::string::npos;
		bool need_coloring = s.var_name=="particles.a" && d_refl;

		if(s.var_name[s.var_name.length()-1]=='\'' && !d_msg)
			continue;

		if(i!=0)
			plot_command << ", ";

		std::string style = this->style == STYLE_LINES ? "lines" : "points ps 0.5";
		if(need_coloring){
			style += " lc variable";
		}
		string title = s.var_name;
		if(title_translation_map.count(title))
			title = title_translation_map[title];
//		if(polar)
//			plot_command << "0.5, ";
		plot_command << "'-' with " << style << " title \"" << title <<"\"";

		// simple field
		if(s.var_name.find('.') == std::string::npos){
			// if need time
			if(getXAxisTime()){
				double val = get_val(msg, d_msg, s.var_name);
				s.data_cache.push_back(make_pair(time, val));

				for(int i=0; i<s.data_cache.size(); i++){
					super_buffer << s.data_cache[i].first << " " << s.data_cache[i].second << "\n";
				}// for
			}// if need time
			else{
				double yval = get_val(msg, d_msg, s.var_name);
				double xval = get_val(msg, d_msg, this->x_axis);
				super_buffer << xval << " " << yval << "\n";
			}// not need time
		}// if simple field
		else{	// repeated field
			std::string f1 = s.var_name.substr(0, s.var_name.find('.'));
				assert(f1.size());
			std::string f2 = s.var_name.substr(s.var_name.find('.')+1);
				assert(f2.size());

			const FieldDescriptor* fd1 = desc->FindFieldByName(f1);
			const FieldDescriptor* d_fd1 = d_desc ? d_desc->FindFieldByName(f1) : NULL;
				assert(!d_desc || fd1);
			int n = refl->FieldSize(*msg, fd1);
				assert(!d_fd1 || refl->FieldSize(*msg, fd1) == d_refl->FieldSize(*msg, d_fd1));

			double prev_x = -100;

			for(int i=0; i<n; i++){
				double y = get_val(&refl->GetRepeatedMessage(*msg, fd1, i), d_refl ? &d_refl->GetRepeatedMessage(*d_msg, d_fd1, i) : NULL, f2);
				double x = i;

				// set x to value of specific x variable if needed
				if(!getXAxisTime()){
					std::string xname = this->x_axis;
					// remove all before dot
					if(this->x_axis.find('.') != std::string::npos)
						xname = x_axis.substr(x_axis.find('.')+1);

					double dop = 0.0;
					//!!!
					if(xname=="psi"){
						dop = get_val(&refl->GetRepeatedMessage(*msg, fd1, i), NULL, "z");
					}

					x = get_val(&refl->GetRepeatedMessage(*msg, fd1, i), d_refl ? &d_refl->GetRepeatedMessage(*d_msg, fd1, i) : NULL, xname) - dop;
				}// if not time :(

				// start new series if needed
				if(need_series_wrap && prev_x>-99 && abs(x-prev_x)>0.5){
					super_buffer << "e\n";
					plot_command << ", '-' with " << style << " title '" << title <<"'";
				}

				if(polar)
					super_buffer << x/*/0.5*M_PI*/ << " " << y;
				else
					super_buffer << x << " " << y;

				if(need_coloring){
					const Message& d_m2 = d_refl->GetRepeatedMessage(*d_msg, d_fd1, i);
					double dy = get_val(NULL, &d_m2, f2+"'");
					if(dy>0)
						super_buffer << " 7";
					else
						super_buffer << " 1";
				}

				super_buffer << "\n";

				prev_x = x;
			}// for points
		}// if repeated field
		super_buffer << "e\n";
	}// for series

	// print all!
	plot_command << "\n";
	fprintf(fp, plot_command.str().c_str());
	fprintf(fp, super_buffer.str().c_str());
	fflush(fp);

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

void Gnuplot::addVar(std::string var){
	serie s;
	s.var_name = var;
	series.push_back(s);
}
void Gnuplot::addExpression(std::string expr){
	serie s;
	s.is_expression = true;

	// replace var names with numbers starting at 2
	char* dup = strdup(expr.c_str());
	char var[45];
	int varno = 2;
	char* p = dup;
	while(p=strchr(p, '$')){
		sscanf(p, "%[$a-zA-Z0-9_.']", var);
		// remember. duplicates are OK
		s.columns.push_back(var+1);	// exclude $ sign
		// remove from p
		int varlen = strlen(var)-1;
		char* beg = strstr(p, var+1);
		*beg = '0'+varno;
		for(char* cur=beg+1; *(cur+varlen-2); cur++){
			*cur = *(cur+varlen-1);
		}// for
		p = beg+1;
		varno++;
	}

	s.var_name = dup;
	series.push_back(s);
	free(dup);
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

std::string Gnuplot::draw_bells(const google::protobuf::Message* msg, const google::protobuf::Descriptor* desc, const google::protobuf::Reflection* refl){
	std::stringstream plot_command;

	const google::protobuf::FieldDescriptor *a0_fd = desc->FindFieldByName("a0");
	double a0 = refl->GetDouble(*msg,a0_fd);
	plot_command << "set object circle at 0,0 size "<< a0 <<"\n";

	const google::protobuf::FieldDescriptor *phi_fd = desc->FindFieldByName("phi");
	double phi = refl->GetDouble(*msg,phi_fd);

	//plot_command << "set arrow 1 from " << a0*cos(phi) << "," << a0*sin(phi) << " to " << -a0*cos(phi) << "," << -a0*sin(phi) << "\n";
	return plot_command.str();
}
