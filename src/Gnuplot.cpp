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

Gnuplot::Gnuplot() {
	width = 400;
	height = 400;
	title = "";
	x_axis = "";			// time/index
	derivative_x = false;
	style = STYLE_LINES;
	polar = 0;

	int rf, wf;
	rpc_call("gnuplot", &rf, &wf);
	to_gnuplot = fdopen(wf, "wb");

	if(!to_gnuplot){
		perror(NULL);
		assert(false);
	}

	update_view();
}

void Gnuplot::processState(const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	printPlotCommand(to_gnuplot, msg, d_msg, time);
}

void Gnuplot::printPlotCommand(FILE* fp, const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	assert(msg);

	const Descriptor* desc = msg->GetDescriptor();
	const Reflection* refl = msg->GetReflection();
	const Descriptor* d_desc = d_msg ? d_msg->GetDescriptor() : NULL;
	const Reflection* d_refl = d_msg ? d_msg->GetReflection() : NULL;

	// exit if we need derivatives but don't have them
//	if(d_msg == NULL || d_msg->ByteSize() <= 32){	// XXX: means it is "nearly empty"
//		return;
//	}

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

		if(s.derivative && !d_msg)
			continue;

		if(i!=0)
			plot_command << ", ";

		std::string style = this->style == STYLE_LINES ? "linespoints" : "points ps 0.2";
		string title = s.var_name;
		if(s.derivative)
			title += '\'';
//		if(polar)
//			plot_command << "0.5, ";
		plot_command << "'-' with " << style << " title \"" << title <<"\"";

		// simple field
		if(s.var_name.find('.') == std::string::npos){
			// if need time
			if(getXAxisTime()){
				const FieldDescriptor* fd = s.derivative ? d_desc->FindFieldByName(s.var_name) : desc->FindFieldByName(s.var_name);
				assert(fd);
				double val = s.derivative ? d_refl->GetDouble(*d_msg, fd) : refl->GetDouble(*msg, fd);
				s.data_cache.push_back(make_pair(time, val));

				for(int i=0; i<s.data_cache.size(); i++){
					super_buffer << s.data_cache[i].first << " " << s.data_cache[i].second << "\n";
				}// for
			}// if need time
			else{
				const FieldDescriptor* yfd = s.derivative ? d_desc->FindFieldByName(s.var_name) : desc->FindFieldByName(s.var_name);
					assert(yfd);
				const FieldDescriptor* xfd = derivative_x ? d_desc->FindFieldByName(this->x_axis) : desc->FindFieldByName(this->x_axis);
					assert(xfd);
				double yval = s.derivative ? d_refl->GetDouble(*d_msg, yfd) : refl->GetDouble(*msg, yfd);
				double xval = derivative_x ? d_refl->GetDouble(*d_msg, xfd) : refl->GetDouble(*msg, xfd);
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
			bool need_series_wrap = !getXAxisTime() && !polar && this->x_axis.find("ksi") != std::string::npos;

			for(int i=0; i<n; i++){
				const Message& m2 = s.derivative ? d_refl->GetRepeatedMessage(*d_msg, d_fd1, i) : refl->GetRepeatedMessage(*msg, fd1, i);
				const Descriptor* d2 = m2.GetDescriptor();
				const Reflection* r2 = m2.GetReflection();
				const FieldDescriptor* fd2 = d2->FindFieldByName(f2);
					assert(fd2);

				double y = r2->GetDouble(m2, fd2);
				double x = i;

				// set x to value of specific x variable if needed
				if(!getXAxisTime()){
					std::string xname = this->x_axis;
					// remove all before dot
					if(this->x_axis.find('.') != std::string::npos)
						xname = x_axis.substr(x_axis.find('.')+1);

					const Message& m2 = derivative_x ? d_refl->GetRepeatedMessage(*d_msg, fd1, i) : refl->GetRepeatedMessage(*msg, fd1, i);
					const Descriptor* d2 = m2.GetDescriptor();
					const Reflection* r2 = m2.GetReflection();
					const FieldDescriptor* xfd = d2->FindFieldByName(xname);
					x = r2->GetDouble(m2, xfd);
				}// if not time :(

				// start new series if needed
				if(need_series_wrap && prev_x>-99 && abs(x-prev_x)>0.5){
					super_buffer << "e\n";
					plot_command << ", '-' with " << style << " title '" << title <<"'";
				}

				if(polar)
					super_buffer << x/0.5*M_PI << " " << y << "\n";
				else
					super_buffer << x << " " << y << "\n";

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
	fprintf(to_gnuplot, "set terminal x11\n");
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
	s.derivative = var[var.size()-1]=='\'';
	s.var_name = s.derivative ? var.substr(0, var.size()-1) : var;
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

Gnuplot::~Gnuplot() {
	fprintf(to_gnuplot, "exit\n");
	fflush(to_gnuplot);
	fclose(to_gnuplot);
}

void Gnuplot::update_view(){
	fprintf(to_gnuplot, "set terminal x11 size %d, %d title \"%s\"\n", width, height, title.c_str());
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
