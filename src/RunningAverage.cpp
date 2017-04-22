/*
 * RunningAverage.cpp
 *
 *  Created on: Apr 13, 2017
 *      Author: dimalit
 */

#include <RunningAverage.h>

RunningAverage::RunningAverage(int size) {
	this->size = size;
	sum = 0;
}

RunningAverage::~RunningAverage() {
}

void RunningAverage::push(double val){
	queue.push(val);
	sum += val;
	if(queue.size() > this->size){
		sum -= queue.front();
		queue.pop();
	}
}
double RunningAverage::get() const {
	return sum / queue.size();
}

