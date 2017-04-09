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
#include <map>
#include <string>

class Gnuplot {
public:
	enum style_enum{STYLE_LINES, STYLE_POINTS};
	static std::map<std::string, std::string> title_translation_map;
private:
	struct serie{
		std::string var_name;			// can contain '
		std::vector<std::pair<std::string, std::string>> data_cache;
		bool is_expression;
		std::string expression;
		std::string prefix;		// for particles.* style vars
		std::vector<std::string> columns;
		serie(){
			is_expression = false;
			expression = "$2";
		}
	};

	FILE* to_gnuplot;
	int x_win_id;
	std::string  title;
	int width, height;
	style_enum style;

	std::string x_axis;
	std::vector<std::string> x_columns;
	std::string x_prefix;
	bool polar;
	bool parametric;
	std::vector<serie> series;
public:
	Gnuplot(int x_win_id=0);

	void processState(const google::protobuf::Message* state, const google::protobuf::Message* d_state = NULL, double time = 0.0);
	void processToFile(const std::string& file, const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time);
	void saveToCsv(const std::string& file, const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time);
	void saveSerie(int serie_no, const std::string& file, const google::protobuf::Message* msg = NULL, const google::protobuf::Message* d_msg = NULL, double time = 0);

	void setXAxisTime(){
		x_axis = "";
		style = STYLE_LINES;
	}
	//!!! ACHTUNG: this can be called ONLY BEFORE addVar()!!
	void setXAxisVar(std::string var);
	bool getXAxisTime() const {
		return x_axis.empty();
	}
	std::string getXAxisVar() const{
		return x_axis;
	}
	bool getPolar() const {
		return polar;
	}
	void setPolar(bool p){
		polar = p;
	}
	bool getParametric() const {
		return parametric;
	}
	void setParametric(bool p){
		parametric = p;
	}

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

	void writeback();
	void restore();

	void setXRange(double from, double to);
	void setYRange(double from, double to);

private:
	void update_view();
	void printPlotCommand(FILE* fp, const google::protobuf::Message* state, const google::protobuf::Message* d_state = NULL, double time = 0.0);
	std::string draw_bells(const google::protobuf::Message*, const google::protobuf::Descriptor*, const google::protobuf::Reflection*);
	void print_numbers(const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time, serie& s, std::ostream& super_buffer, bool add_to_cache = true);
};

#endif /* GNUPLOT_H_ */
