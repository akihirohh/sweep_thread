#include "sweepRead.h"

namespace sweepRead
{	
	/*
	bool isLidarOpen(Urg_driver& urg)
	{
		return urg.is_open();
	}
	void closeLidar(Urg_driver& urg)
	{
		urg.close();
	}
	int initLidar(sweep::sweep &device, std::string portname, int motor_speed)
	{
	}

	void getLidar(Urg_driver& urg, std::vector<long> &data_distance, std::vector<unsigned short> &data_intensity)
	{
		long time_stamp = 0;
		int success = 0;
		success = urg.get_distance_intensity(data_distance, data_intensity, &time_stamp);
		if(!success)
		{
				std::cout << "Urg_driver::get_distance(): " << urg.what() << std::endl;
				data_distance.push_back(-1);
		}     
		else
		{
				data_distance.push_back(time_stamp);
		}
	}*/
	void *sweepReading( void *ptr )
	{
		thdata *data;
		data = (thdata *) ptr;
		int b_loop = 1, cnt = 0;	
		std::vector<long> distance;
		std::vector<double> angle;
		std::vector<unsigned short> data_intensity;

		data->mtx.lock();
		sweep::sweep device{data->ip_or_portname.c_str()};
		device.set_motor_speed(5);
		std::cout << "Motor Speed Setting: " << device.get_motor_speed() << "Hz" << std::endl;
		std::cout << "Sample Rate Setting: " << device.get_sample_rate() << "Hz" << std::endl; 
		std::cout << "Beginning data acquisition as soon as motor speed stabilizes..." << std::endl;
		device.start_scanning();
		data->b_loop = 1;
		data->mtx.unlock();

		while(b_loop)
		{
			try
			{
				const sweep::scan scan = device.get_scan();
				distance.clear();
				angle.clear();
				data_intensity.clear();				
				for (const sweep::sample& sample : scan.samples) 
				{
				  distance.push_back(sample.distance*10);
				  angle.push_back((double)sample.angle/1000);
				  data_intensity.push_back(sample.signal_strength);

				  std::cout << "angle " << (double)sample.angle/1000 << " distance " << sample.distance*10 << " strength " << sample.signal_strength << "\n";
				}
				std::cout << "\nSizes: lidar: " << distance.size() << "\tangle: " << angle.size();
				cnt++;
				
			}
			catch (...)
			{
				std::cout << "\nERROR...";     
			}  				
			data->mtx.lock();
			data->distance.clear();
			data->intensity.clear();
			data->reading_angle.clear();
			std::copy(distance.begin(), distance.end(), std::back_inserter(data->distance));
			std::copy(data_intensity.begin(), data_intensity.end(), std::back_inserter(data->intensity));
			std::copy(angle.begin(), angle.end(), std::back_inserter(data->reading_angle));
			data->timestamp = cnt;	
			if(b_loop) b_loop = data->b_loop;
			else data->b_loop = b_loop;
			data->mtx.unlock();
			if(cnt > 10000) cnt = 0;
		}
		device.stop_scanning();
		device.set_motor_speed(0);
		std::cout << "\nSweep Stopped...";
	}	
}
