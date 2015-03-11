#include "StatFile.hpp"

#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>

void StatFile::open(const char* filename)
{
  _ofs.open(filename, std::ofstream::out);
}
void StatFile::close()
{
  _ofs.close();
}

void StatFile::add(int measure, double value)
{
  _ofs << measure << '\t' << value << std::endl;
}

void StatFile::clearDirectory(const char* directory)
{
  remove(directory);
  mkdir(directory, S_IRUSR | S_IWUSR | S_IXUSR);
}
