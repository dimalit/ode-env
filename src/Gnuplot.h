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
		std::vector<double> data_cache;
	};

	FILE* to_gnuplot;
	std::string  title;
	int width, height;
	style_enum style;

	std::string x_axis;
	std::vector<serie> series;
public:
	Gnuplot();

	void processState(const google::protobuf::Message*);

	void setXAxisTime(){
		x_axis = "";
	}
	void setXAxisVar(std::string var){
		x_axis = var;
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

private:
	void update_view();
};

#endif /* GNUPLOT_H_ */
