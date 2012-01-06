#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <tr1/array>
#include <vector>

#include <sys/inotify.h>
#include <unistd.h>

#include "uvf.h"

// grabbed these from agent.inp...
const std::tr1::array<float, 27> zdist = {{
  3, 3, 3, 3, 3,  4, 4, 5, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  5, 5,  6,
  2, 2, 2, 2, 2
}};
std::tr1::array<float, 27> zmemo;

const size_t NUM_COLUMNS = 11;

// miserably inefficient way to do this.
void read_column(std::istream& is, size_t col, std::vector<float>& into)
{
  float tmp, garbage;
  into.clear();
  do {
    for(size_t i=0; i < col; ++i) { is >> garbage; }
    is >> tmp;
    for(size_t i=col; i < NUM_COLUMNS; ++i) { is >> garbage; }
    if(is) {
      into.push_back(tmp);
    }
  } while(is);
}

// each column in the input file is a different variable.  this maps the column
// index to the name of the variable.
std::string variable_name(size_t column) {
  switch(column) {
    case 0: return "x-coord";
    case 1: return "y-coord";
    case 2: return "flux 1";
    case 3: return "flux 2";
    case 4: return "flux 3";
    case 5: return "flux 4";
    case 6: return "flux 5";
    case 7: return "flux 6";
    case 8: return "flux 7";
    case 9: return "absorption";
    case 10: return "fission";
    default: return "unknown";
  }
}

void gen_raw(std::vector<std::string> files, size_t column,
             const size_t dims[3],
             std::ostream& bin)
{
  std::vector<float> d;

  std::cout << "Generating RAW data for variable '" << variable_name(column)
            << "' ...\n";
  d.reserve(files.size() * 360000);
  for(std::vector<std::string>::const_iterator f = files.begin();
      f != files.end(); ++f) {
    std::cout << "  .. from file " << *f << "... ";
    std::vector<float> tmp;
    std::ifstream ifs(f->c_str(), std::ios::in);
    read_column(ifs, column, tmp);
    d.insert(d.end(), tmp.begin(), tmp.end());
    std::cout << d.size() << " elems thus far.\n";
  }
  assert(d.size() ==
         std::accumulate(dims, dims+3, static_cast<size_t>(1),
                         std::multiplies<size_t>()));
  bin.write(reinterpret_cast<char*>(&d[0]), d.size()*sizeof(float));
}

class IterationUnchanged : public std::runtime_error {
public:
  IterationUnchanged(std::string s) : std::runtime_error(s) {}
};

void read_parameters(const char* metadata, size_t& iteration,
                     size_t dims[3], std::vector<std::string>& files)
{
  files.clear();
  std::ifstream is(metadata, std::ios::in);
  if(!is) {
    throw std::runtime_error("Could not open metadata file!");
  }
  {
    size_t iter;
    is >> iter;
    if(is.fail()) {
      throw std::runtime_error("Could not read iteration number.");
    }
    if(iter == iteration) {
      throw IterationUnchanged("Iteration has not changed.");
    }
    iteration = iter;
  }

  {
    is >> dims[0] >> dims[1] >> dims[2];
    if(is.fail()) {
      throw std::runtime_error("Could not read dimensions.");
    }
  }
  do {
    std::string file;
    is >> file;
    if(is) { files.push_back(file); }
  } while(is);
  is.close();
}

static void wait_for_event(int fd) {
  struct inotify_event event;
  std::cout << "Waiting until metadata file changes...\n";
  read(fd, &event, sizeof(struct inotify_event));
}
static void convert_to_binary(const std::vector<std::string>& files,
                              const size_t dims[3], const char* rawfn)
{
  std::ofstream bin(rawfn, std::ios::binary | std::ios::trunc);
  gen_raw(files, 2, dims, bin); // 3rd column (index 2) == flux_1
  bin.close();
}

int main(int argc, char *argv[]) {
  if(argc == 1) {
    std::cerr << "Need metadata file.\n";
    exit(EXIT_FAILURE);
  }

  int inotify = inotify_init1(IN_CLOEXEC);
  int watch = inotify_add_watch(inotify, argv[1], IN_CLOSE_WRITE);

  size_t iteration;
  size_t dims[3];
  std::vector<std::string> files;

  const char* tmpraw = ".out.raw";
  do {
    wait_for_event(inotify);
    try {
      read_parameters(argv[1], iteration, dims, files);
      convert_to_binary(files, dims, tmpraw);
      uvf_convert(tmpraw, "in-place.uvf", dims);
      unlink(tmpraw);
    } catch(const IterationUnchanged&) {
      std::cout << "Iteration variable did not change.  Skipping this event.\n";
    } catch(const std::runtime_error& err) {
      std::cerr << "error: " << err.what() << "\nSkipping this event.\n";
    }
  } while(1);

  if(inotify_rm_watch(inotify, watch) != 0) {
    std::cerr << "Error deleting watch: " << errno << "\n";
  }
  close(watch);
  close(inotify);

  return 0;
}
