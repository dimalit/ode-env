/*
 * Gnuplot.h
 *
 *  Created on: Jan 25, 2015
 *      Author: dimalit
 */

#ifndef GNUPLOT_H_
#define GNUPLOT_H_

#include <google/protobuf/message.h>

#include <vector>
#include <string>

class Gnuplot {
public:
	enum style_enum{STYLE_LINES, STYLE_POINTS};
private:
	struct serie{
		std::string var_name;
		std::vector<std::pair<double, double>> data_cache;
		bool derivative;
	};

	FILE* to_gnuplot;
	std::string  title;
	int width, height;
	style_enum style;

	std::string x_axis;
	bool derivative_x;
	std::vector<serie> series;
public:
	Gnuplot();

	void processState(const google::protobuf::Message* state, const google::protobuf::Message* d_state = NULL, double time = 0.0);
	void processToFile(const std::string& file, const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time);

	void setXAxisTime(){
		x_axis = "";
	}
	void setXAxisVar(std::string var){
		derivative_x = var.size() > 0 && var[var.size()-1]=='\'';
		std::string real_var = derivative_x ? var.substr(0, var.size()-1) : var;
		x_axis = real_var;
	}
	bool getXAxisTime() const {
		return x_axis.empty();
	}
	std::string getXAxisVar() const;

	void addVar(std::string var);
	void eraseVar(int idx);
	int getVarsCount() const {
		return series.size();
	}

	void reset(){
		for(int i=0; i<series.size(); i++)
			series[i].data_cache.clear();
	}

	virtual ~Gnuplot();

	//TODO: update on each change

	void setTitle(std::string title){
		this->title = title;
		update_view();
	}
	std::string getTitle() const {
		return title;
	}
	int getWidth() const {
		return width;
	}
	int getHeight() const {
		return height;
	}
	void setSize(int w, int h){
		width = w;
		height = h;
		update_view();
	}
	style_enum getStyle() const {return style;}
	void setStyle(style_enum s) {this->style = s;}

	void writeback() const;
	void restore() const;

private:
	void update_view();
};

#endif /* GNUPLOT_H_ */
