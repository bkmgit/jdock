#pragma once
#ifndef IDOCK_STOPWATCH_HPP
#define IDOCK_STOPWATCH_HPP

#include <vector>
#include <array>
#include <chrono>
#include <string>
using namespace std;
using namespace std::chrono;

//! Represents a stopwatch.
class stopwatch
{
public:
	//! Constructs a standby stopwatch.
	explicit stopwatch();

	static stopwatch start_new();

	void start();

	void stop();

	void restart();

	void reset();

	inline bool is_running();

	//!< Accumulated elapsed nanoseconds.
	long long elapsed();

	//!< Accumulated elapsed seconds.
	double elapsed_sec();

	//!< Accumulated elapsed time in mm:ss.sss format.
	string elapsed_str();

private:
	static const high_resolution_clock clock; //!< A system high resolution clock to query current time.
	size_t elapsed_ns; //!< Accumulated elapsed nanoseconds.
	time_point<steady_clock> started_time; //!< Started time of the current running.
	bool running; //!< Current running status.
};

#endif
