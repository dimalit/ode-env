/*
 * Gnuplot.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: dimalit
 */

#include "Gnuplot.h"
#include "rpc.h"

#include <sstream>
#include <cstdio>

using namespace google::protobuf;

Gnuplot::Gnuplot() {
	width = 300;
	height = 300;
	title = "";
	x_axis = "";			// time/index
	style = STYLE_LINES;

	int rf, wf;
	rpc_call("gnuplot", &rf, &wf);
	to_gnuplot = fdopen(wf, "wb");

	if(!to_gnuplot){
		perror(NULL);
		assert(false);
	}

	update_view();
}

void Gnuplot::processState(const google::protobuf::Message* msg){
	assert(msg);

	const Descriptor* desc = msg->GetDescriptor();
	const Reflection* refl = msg->GetReflection();

	//////////////// 1 plot xxx with xxx, yyy with yyy /////////////////

	std::ostringstream plot_command;
	plot_command << "plot ";

	for(int i=0; i<series.size(); i++){
		const serie& s = series[i];
		if(i!=0)
			plot_command << ", ";

		std::string style = this->style == STYLE_LINES ? "lines" : "dots";
		plot_command << "'-' with " << style << " title '" << s.var_name <<"'";
	}// for
	plot_command << "\n";
	fprintf(to_gnuplot, plot_command.str().c_str());
	fflush(to_gnuplot);

	//////////// 2 give series ////////////////////
	for(int i=0; i<series.size(); i++){
		serie& s = series[i];

		// simple field
		if(s.var_name.find('.') == std::string::npos){
			// if need time
			if(getXAxisTime()){
				const FieldDescriptor* fd = desc->FindFieldByName(s.var_name);
				assert(fd);
				double val = refl->GetDouble(*msg, fd);
				s.data_cache.push_back(val);

				for(int i=0; i<s.data_cache.size(); i++){
					fprintf(to_gnuplot, "%lf %lf\n", (double)i, s.data_cache[i]);
				}// for
			}// if need time
			else{
				const FieldDescriptor* yfd = desc->FindFieldByName(s.var_name);
					assert(yfd);
				const FieldDescriptor* xfd = desc->FindFieldByName(this->x_axis);
					assert(xfd);
				double yval = refl->GetDouble(*msg, yfd);
				double xval = refl->GetDouble(*msg, xfd);
				fprintf(to_gnuplot, "%lf %lf\n", xval, yval);
			}// not need time
		}// if simple field
		else{	// repeated field
			std::string f1 = s.var_name.substr(0, s.var_name.find('.'));
				assert(f1.size());
			std::string f2 = s.var_name.substr(s.var_name.find('.')+1);
				assert(f2.size());

			const FieldDescriptor* fd1 = desc->FindFieldByName(f1);
				assert(fd1);
			int n = refl->FieldSize(*msg, fd1);

			for(int i=0; i<n; i++){
				const Message& m2 = refl->GetRepeatedMessage(*msg, fd1, i);
				const Descriptor* d2 = m2.GetDescriptor();
				const Reflection* r2 = m2.GetReflection();
				const FieldDescriptor* fd2 = d2->FindFieldByName(f2);
					assert(fd2);

				double y = r2->GetDouble(m2, fd2);
				double x = i;

				// set x to value of specific x variable if needed
				if(!getXAxisTime()){
					std::string xname = this->x_axis;
					if(this->x_axis.find('.') != std::string::npos)
						xname = x_axis.substr(x_axis.find('.')+1);

					const FieldDescriptor* xfd = d2->FindFieldByName(xname);
					x = r2->GetDouble(m2, xfd);
				}// if not time :(

				fprintf(to_gnuplot, "%lf %lf\n", x, y);
			}// for
		}// if repeated field

		fprintf(to_gnuplot, "e\n");
		fflush(to_gnuplot);
	}// for
}

void Gnuplot::addVar(std::string var){
	serie s;
	s.var_name = var;
	series.push_back(s);
}
void Gnuplot::eraseVar(int idx){
	assert(idx >= 0 && idx <series.size());
	series.erase(series.begin()+idx);
}

Gnuplot::~Gnuplot() {
	fprintf(to_gnuplot, "exit\n");
	fflush(to_gnuplot);
	fclose(to_gnuplot);
}

void Gnuplot::update_view(){
	fprintf(to_gnuplot, "set terminal x11 size %d, %d title \"%s\"\n", width, height, title.c_str());
	fflush(to_gnuplot);
}
