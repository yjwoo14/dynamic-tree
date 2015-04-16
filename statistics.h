#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include <limits>

// TODO: Write down the limit of statistic functions
// Also if the statistic is for evertable link cut tree then,
// UpdateLeft and UpdateRight should behave same.

// Basic Statistic 
// Computing subtree size and keeping the same key in the same node
class Statistic {
	public:
	size_t cnt;
	
	// Essential Functions for Statistic
	Statistic() : cnt(0) {}
	bool Exist() {
		return cnt > 0;
	}

	void Add() {
		cnt++;
	}

	void Remove() {
		cnt--;
	}

	template <typename T>
	void Init(const T& key) {}

	template <typename ST>
	void UpdateLeft(const ST& s) {}

	template <typename ST>
	void UpdateRight(const ST& s) {}
};

class SubtreeSizeStatistic: public Statistic {
	public:
	size_t ss;
	SubtreeSizeStatistic()
		: Statistic(),
		ss(0) {}
	template <typename T>
	void Init(const T& key) {
		ss = cnt;
	}

	void UpdateLeft(const SubtreeSizeStatistic& s) {
		Update(s);
	}

	void UpdateRight(const SubtreeSizeStatistic& s) {
		Update(s);
	}

	void Update(const SubtreeSizeStatistic& s) {
		ss += s.ss;
	}
};

template <typename T>
class MinMaxStatistic : public Statistic {
	public:
	T min_weight;
	T max_weight;
	MinMaxStatistic() 
		: Statistic(), 
		min_weight(std::numeric_limits<T>::max()),
		max_weight(std::numeric_limits<T>::min()) {}

	void Init(const T& key) {
		min_weight = key;
		max_weight = key;
	}

	void UpdateLeft(const MinMaxStatistic& s) {
		Update(s);
	}

	void UpdateRight(const MinMaxStatistic& s) {
		Update(s);
	}

	void Update(const MinMaxStatistic& s) {
		min_weight = std::min(min_weight, s.min_weight);
		max_weight = std::max(max_weight, s.max_weight);
	}
};

template <typename T>
class SumStatistic : public Statistic {
	public:
	T sum;
	SumStatistic() : Statistic() {}
	void Init(const T& key) {
		sum = key;
		Statistic::Init(key);
	}

	void UpdateLeft(const SumStatistic& s) {
		Update(s);
	}

	void UpdateRight(const SumStatistic& s) {
		Update(s);
	}

	void Update(const SumStatistic& s) {
		sum += s.sum;
	}
};

#endif
