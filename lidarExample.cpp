#include <iostream>
#include <fstream>
#include <sys/time.h> //gettimeofday
#include <unistd.h>   // UNIX standard function definitions 
#include <errno.h>    // Error number definitions 
#include <termios.h>  // POSIX terminal control definitions 
#include <sys/ioctl.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>

#include "sweepRead.h"

//For data logging purpose. It returns a string
//with current time to uniquely name log files
const std::string currentDateTime(){
	time_t		now = time(0);
	struct tm	tstruct;
	char		buf[80];
	tstruct = *localtime(&now);
	strftime(buf,sizeof(buf), "%Y%m%d_%Hh%Mm%Ss",&tstruct);
	return buf;
}
const std::string currentTime(){
	time_t		now = time(0);
	struct tm	tstruct;
	char		buf[80];
	tstruct = *localtime(&now);
	strftime(buf,sizeof(buf), "%Y%m%d,%H%M%S,", &tstruct);
	return buf;
}
int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}
int millis(timeval t_start)
{
	struct timeval t;
	gettimeofday(&t,NULL);
	return (t.tv_sec - t_start.tv_sec)*1000 + (t.tv_usec - t_start.tv_usec)/1000;	
}

int main(int argc, char *argv[])
{
	//DATA_LOGGING
	std::string lidar_filename, intensity_filename, angle_filename;
	std::fstream lidar_file, intensity_file, angle_file;

	//LIDAR
	int previous_lidar_ts = 0, current_ts = 0;
  std::vector<long> lidar_distance;
  std::vector<unsigned short> lidar_intensity;
  std::vector<double> lidar_angle;
	pthread_t sweep_thread;
	sweepRead::thdata lidar_data;
	lidar_data.b_loop = 0;
  lidar_data.ip_or_portname = "/dev/ttyUSB0";
  lidar_data.connection_type = "-s";

  //MISC
  int loop = 1;
  std::string timestamp;
  struct timeval t_start;

  lidar_filename.append(currentDateTime());
  intensity_filename = lidar_filename;
  angle_filename = lidar_filename;
  lidar_filename.append("_lidar_ts.txt");
  intensity_filename.append("_intensity_ts.txt");
  angle_filename.append("_angle.txt");
  lidar_file.open(lidar_filename.c_str(), std::ios_base::out);
  intensity_file.open(intensity_filename.c_str(), std::ios_base::out);
  angle_file.open(angle_filename.c_str(), std::ios_base::out);

  pthread_create (&sweep_thread, NULL, &sweepRead::sweepReading, &lidar_data);
  while (!lidar_data.b_loop);

  gettimeofday(&t_start,NULL);

  while(loop)
  {    
    timestamp = std::to_string(millis(t_start));
    try
    {
      lidar_distance.clear();
      lidar_intensity.clear();
      lidar_angle.clear();
      lidar_data.mtx.lock();
      std::copy(lidar_data.distance.begin(), lidar_data.distance.end()-1, std::back_inserter(lidar_distance));
      std::copy(lidar_data.intensity.begin(), lidar_data.intensity.end(), std::back_inserter(lidar_intensity));
      std::copy(lidar_data.reading_angle.begin(), lidar_data.reading_angle.end(), std::back_inserter(lidar_angle));
      current_ts = lidar_data.timestamp;
      lidar_data.mtx.unlock();
    }
    catch (...) { std::cout << "\n\nCouldn't copy latest LiDAR readings...\n\n";}
    
    if(current_ts != previous_lidar_ts)
    {
      previous_lidar_ts = current_ts;
      std::cout << std::endl << timestamp << "\nLidar samples:\n";
      for (int i = 0; i < lidar_distance.size(); i += lidar_distance.size()/10)
      {
        std::cout << "[" << i << "]: " << lidar_distance[i] << " | "  << lidar_intensity[i] << "\t";;
      }
      angle_file << timestamp;
      for (int i = 0; i < lidar_angle.size(); i++) //last one is lidar_timestamp
        angle_file << "," << lidar_angle[i];
      angle_file << std::endl;

      lidar_file << timestamp;
      for (int i = 0; i < lidar_distance.size(); i++) //last one is lidar_timestamp
        lidar_file << "," << lidar_distance[i];
      lidar_file << std::endl;

      intensity_file << timestamp;
      for (int i = 0; i < lidar_distance.size(); i++) //last one is lidar_timestamp
        intensity_file << "," << lidar_intensity[i];
      intensity_file << std::endl;
    }    
    if(kbhit() && getchar() == 'q') loop = 0;
  }
  lidar_data.mtx.lock();
  lidar_data.b_loop = 0;
  lidar_data.mtx.unlock();
  sleep(1);
  std::cout << "\nloop = " << lidar_data.b_loop;
  angle_file.close();
  lidar_file.close();
  intensity_file.close();
  std::cout << lidar_filename << std::endl; 
}