/*
 * MaxDetector.h
 *
 *  Created on: Mar 4, 2016
 *      Author: dimalit
 */

#ifndef MAXDETECTOR_H_
#define MAXDETECTOR_H_

#include <cassert>
#include <limits>

class MaxDetector {
private:
	double v[3];
	int size;
	int processed_count;
public:
	MaxDetector(){
		reset();
	}
	bool push(double val){
		shift();
		v[2] = val;
		if(size<3)
			size++;
		processed_count++;
		return hasMax();
	}
	void reset(){
		size = 0;
		processed_count = 0;
	}
	bool hasMax() const {
		return (size==3 && v[1] > v[0] && v[1] > v[2]);
	}
	double getMax() const {
		assert(hasMax());
		return v[1];
	}
	double get(int i) const {
		return v[i];
	}
	int getSize() const {
		return size;
	}
	double getVariance() const {
		double max = -std::numeric_limits<double>::infinity();
		double min = +std::numeric_limits<double>::infinity();
		for(int i=0; i<size; i++){
			if(v[i] > max)
				max = v[i];
			if(v[i] < min)
				min = v[i];
		}
		return max-min;
	}
	int getProcessed() const {
		return processed_count;
	}
	bool isIncreasing() const {
		return (size<=1 || size==2 && v[0] <= v[1] || size==3 && v[0] <= v[1] && v[1] <= v[2]);
	}
	bool isDecreasing() const {
		return (size<=1 || size==2 && v[0] >= v[1] || size==3 && v[0] >= v[1] && v[1] >= v[2]);
	}
private:
	void shift(){
		v[0] = v[1];
		v[1] = v[2];
	}
};

#endif /* MAXDETECTOR_H_ */
