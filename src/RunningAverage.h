/*
 * RunningAverage.h
 *
 *  Created on: Apr 13, 2017
 *      Author: dimalit
 */

#ifndef RUNNINGAVERAGE_H_
#define RUNNINGAVERAGE_H_

#include <queue>

class RunningAverage {
public:
	RunningAverage(int size);
	~RunningAverage();
	void push(double val);
	double get() const;
private:
	double sum;
	int size;
	std::queue<double> queue;
};

#endif /* RUNNINGAVERAGE_H_ */
