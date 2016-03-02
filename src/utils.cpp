#include "utils.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>



std::string utils::extractFileNameFromPath(std::string path){
  // Remove directory if present.
  // Do this before extension removal incase directory has a period character.
  std::string out_path;
  const size_t last_slash_idx = path.find_last_of( FILE_SLASH );
  if (std::string::npos != last_slash_idx){ path.erase(0, last_slash_idx + 1); }

  // Remove extension if present.
  const size_t period_idx = path.rfind('.');
  if (std::string::npos != period_idx){ path.erase(period_idx); }
  out_path = path;
  return out_path;
}

std::string utils::extractFileNameFromPath(std::string path, std::string &ext){
  // Remove directory if present.
  // Do this before extension removal incase directory has a period character.
  std::string out_path;
  const size_t last_slash_idx = path.find_last_of( FILE_SLASH );
  if (std::string::npos != last_slash_idx){ path.erase(0, last_slash_idx + 1); }

  // Return extension if present.
  std::vector<std::string> out;
  split(path, '.', out);
  if( out.size() > 1 ){
    ext = out[1];
    out_path = out[0];
  }else{
    ext = "";
    out_path = "";
  }
  return out_path;
}


std::string utils::extractFileFolderFromPath(std::string path){
  // Remove directory if present.
  // Do this before extension removal incase directory has a period character.
  const size_t last_slash_idx = path.find_last_of( FILE_SLASH );
  if (std::string::npos != last_slash_idx){ path.erase(last_slash_idx, path.size() - last_slash_idx); }

  return path;
}


void utils::split(const std::string &s, char delim, std::vector<std::string> &elems){
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    if (item.length() > 0) {
      elems.push_back(item);  
    }
  }
}


std::string utils::float2str(float num){
  std::stringstream sstr;
  std::string result;
  sstr << std::setprecision(6) << num;
  result = sstr.str();
  return result;
}

std::string utils::float2str(float num, uint floatpres){
  std::stringstream sstr;
  std::string result;
  if(num == 0){
    sstr << 0;
  }else{
    sstr << std::fixed << std::setprecision(floatpres) << num;
  }
  result = sstr.str();
  return result;
}

std::string utils::double2str(double num){
  std::stringstream sstr;
  std::string result;
  sstr << std::setprecision(6) << num;
  result = sstr.str();
  return result;
}

std::string utils::int2str(int num){
  std::stringstream sstr;
  sstr << num;
  return sstr.str();
}

std::string utils::uint2str(uint num){
  std::stringstream sstr;
  sstr << num;
  return sstr.str();
}
