#ifndef __STATFILE_H__
#define __STATFILE_H__

#include <fstream>

class StatFile
{
 private:

  std::ofstream _ofs;

 public:

  void open(const char* filename);
  void close();

  void add(int measure, double value);

  static void clearDirectory(const char* directory);
};

#endif
