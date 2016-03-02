#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <string>
#include <vector>

const std::string GAZETRACKER_VERSION="0.1";


//#define FILE_SLASH "//"
#ifdef __linux__
//linux operative system
#define FILE_SLASH "//"
#elif __APPLE__
  //macintosh operative system
typedef unsigned int uint;
#define FILE_SLASH "//"
#else
  //Windows operative system
#define FILE_SLASH "/\\"
#define uint unsigned int
#endif

namespace utils
{
  std::string   extractFileNameFromPath(std::string path);
  std::string   extractFileNameFromPath(std::string path, std::string &ext);
  std::string   extractFileFolderFromPath(std::string path);
  void          split(const std::string &s, char delim, std::vector<std::string> &elems);
  std::string   float2str(float number);
  std::string   float2str(float num, uint floatpres);
  std::string   double2str(double num);
  std::string   int2str(int num);
  std::string   uint2str(uint num);
};

#endif /* UTILS_HPP_ */
