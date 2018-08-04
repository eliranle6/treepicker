#ifndef __GETEXEPATH_H
#define __GETEXEPATH_H

#include <linux/limits.h>
#include <unistd.h>
#include <string>
#include <iostream>

#ifdef WIN32
std::string getexepath()
{
	char result[MAX_PATH];
	uint32_t found;
	GetModuleFileName( NULL, result, MAX_PATH );
	found = std::string(result).find_last_of("\\");
	return(std::string(result).substr(0,found) + "\\");
}
#else
std::string getexepath()
{
	char result[ PATH_MAX ];
	ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
	//found = std::string(result).find_last_of("/");
	return std::string( result, (count > 0) ? count : 0 );
}
#endif

#endif // if WIN32
