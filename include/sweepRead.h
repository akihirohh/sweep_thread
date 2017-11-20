#ifndef SWEEPREAD_H_
#define SWEEPREAD_H_

#include <iostream>
#include <mutex>
#include <pthread.h>
#include <vector>
#include <cstring>

#include <sweep/sweep.hpp>

namespace sweepRead
{	
	typedef struct str_thrdata
	{
		int b_loop;
		std::vector<long> distance;	//vector of read distances
		std::vector<unsigned short> intensity;
		std::vector<double> reading_angle;
		int timestamp;
		std::string ip_or_portname;
		std::string connection_type;
		std::mutex mtx;
	} thdata;
	/*
	void 	closeLidar(Urg_driver& urg);
	void	getLidar(Urg_driver& urg, std::vector<long> &data_distance, std::vector<unsigned short> &data_intensity);
	int	    initLidar(int argc, const char **argv, Urg_driver& urg);
	bool 	isLidarOpen(Urg_driver& urg);*/
	void 	*sweepReading(void *ptr); 
}

#endif