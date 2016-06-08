/*******************************************************************\

           Copyright (c) 2009-2016 Cauldron Development LLC

    This program is free software; you can redistribute it and/or
     modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
        of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
             GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
                           02111-1307, USA.

       Website: http://cauldrondevelopment.com/
       Repository: https://cauldrondevelopment.com/svn/rlimit
       Author: Joseph Coffland <joseph@cauldrondevelopment.com>

\*******************************************************************/


#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <sstream>

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/resource.h>

using namespace std;

#define THROW(x) do {                                                   \
    ostringstream __str; __str << x; throw runtime_error(__str.str());  \
  } while (0)


int verbosity = 0;
bool human = false;


struct Resource {
  int id;
  const char *name;
  const char *longOpt;
  char shortOpt;

  rlim_t stringToLimit(const string &limit) const {
    const char *s = limit.c_str();
    char *endptr = 0;

    errno = 0;
    rlim_t x = (rlim_t)strtol(s, &endptr, 0);
    if (errno || !endptr) THROW("Error parsing limit value '" << limit << "'");

    switch (*endptr) {
    case 'k': x *= 1000; break;
    case 'm': x *= 1000000; break;
    case 'g': x *= 1000000000; break;
    case 'K': x <<= 10; break;
    case 'M': x <<= 20; break;
    case 'G': x <<= 30; break;
    case 0: break;
    default: THROW("Invalid limit '" << limit << "'");
    }

    return (rlim_t)x;
  }

  const string limitToString(rlim_t value) const {
    if (value == RLIM_INFINITY) return "unlimited";
    ostringstream str;

    if (human) {
      double x = value;
      char c = 0;

      if (1 << 30 <= value) {x /= 1 << 30; c = 'G';}
      else if (1 << 20 <= value) {x /= 1 << 20; c = 'M';}
      else if (1 << 10 <= value) {x /= 1 << 10; c = 'K';}

      if (c) str << setprecision(2) << fixed << x << c;
      else str << value;

    } else str << value;

    return str.str();
  }

  void doLimit(const char *value = 0) {
    struct rlimit rlimit;

    // Always get first
    if (getrlimit(id, &rlimit)) THROW("Getting " << name << " limit");

    if (value) {
      string s = value;
      size_t comma = s.find(',');
      string soft;
      string hard;

      if (comma != string::npos) {
        soft = s.substr(0, comma);
        hard = s.substr(comma + 1);

      } else soft = s;

      if (soft != "") rlimit.rlim_cur = stringToLimit(soft);
      if (hard != "") rlimit.rlim_max = stringToLimit(hard);

      if (rlimit.rlim_max < rlimit.rlim_cur) rlimit.rlim_cur = rlimit.rlim_max;

      if (setrlimit(id, &rlimit))
        THROW("Setting " << name << " limits to soft="
              << rlimit.rlim_cur << " hard=" << rlimit.rlim_max);

      // Get it again
      if (getrlimit(id, &rlimit)) THROW("Getting " << name << " limit");
    }

    // Print current value
    if (verbosity || !value)
      cout << setw(18) << name << ": "
           << "soft " << setw(10) << limitToString(rlimit.rlim_cur)
           << "  hard " << setw(10) << limitToString(rlimit.rlim_max)
           << endl;
  }
};


Resource resources[] = {
#ifdef RLIMIT_AS
  {RLIMIT_AS,         "Address Space",      "address-space",'a'},
#endif
#ifdef RLIMIT_CORE
  {RLIMIT_CORE,       "Core Size",          "core", 'C'},
#endif
#ifdef RLIMIT_CPU
  {RLIMIT_CPU,        "CPU Time",           "cpu", 'c'},
#endif
#ifdef RLIMIT_DATA
  {RLIMIT_DATA,       "Data Segment Size",  "data", 'd'},
#endif
#ifdef RLIMIT_FSIZE
  {RLIMIT_FSIZE,      "File Size",          "file-size", 'f'},
#endif
#ifdef RLIMIT_LOCKS
  {RLIMIT_LOCKS,      "File Locks",         "locks", 'l'},
#endif
#ifdef RLIMIT_MEMLOCK
  {RLIMIT_MEMLOCK,    "Locked Memory",      "mem-lock", 'L'},
#endif
#ifdef RLIMIT_MSGQUEUE
  {RLIMIT_MSGQUEUE,   "Message Queue",      "message-queue", 'q'},
#endif
#ifdef RLIMIT_NICE
  {RLIMIT_NICE,       "Nice",               "nice", 'n'},
#endif
#ifdef RLIMIT_NOFILE
  {RLIMIT_NOFILE,     "File Descriptors",   "file-descriptors", 'D'},
#endif
#ifdef RLIMIT_NPROC
  {RLIMIT_NPROC,      "Processes",          "processes", 'p'},
#endif
#ifdef RLIMIT_RSS
  {RLIMIT_RSS,        "Resident Set",       "resident-set", 'r'},
#endif
#ifdef RLIMIT_RTPRIO
  {RLIMIT_RTPRIO,     "Real-Time Priority", "rt-priority", 'R'},
#endif
#ifdef RLIMIT_RTTIME
  {RLIMIT_RTTIME,     "Real-Time CPU Time", "rt-time", 'T'},
#endif
#ifdef RLIMIT_SIGPENDING
  {RLIMIT_SIGPENDING, "Pending Signals",    "pending-signals", 'S'},
#endif
#ifdef RLIMIT_STACK
  {RLIMIT_STACK,      "Stack Size",         "stack", 's'},
#endif
};

#define NUM_RESOURCES (sizeof(resources) / sizeof(Resource))


Resource *find_resource(int id) {
  for (unsigned i = 0; i < NUM_RESOURCES; i++)
    if (id == resources[i].id) return &resources[i];

  return 0;
}


Resource *opt_to_resource(int opt) {
  for (unsigned i = 0; i < NUM_RESOURCES; i++)
    if (opt == resources[i].shortOpt) return &resources[i];

  return 0;
}


void usage(ostream &stream, const char *name) {
  stream
    << "Usage: " << name << " [OPTIONS] [COMMAND]\n"
    << "OPTIONS:\n";

  for (unsigned i = 0; i < NUM_RESOURCES; i++) {
    Resource &resource = resources[i];

    stream << "  --" << resource.longOpt << "[=soft[,hard]] | "
           << "-" << resource.shortOpt << "[soft[,hard]]\n"
           << string(50, ' ') << "Get or set " << resource.name << " limit.\n";
  }

  stream
    << "  -v\n" << string(50, ' ') << "Increase verbosity.\n"
    << "  -h\n" << string(50, ' ') << "Print values in human readable form.\n"
    << "  --help\n" << string(50, ' ') << "Print this help screen and exit.\n"
    << "  --version | -V\n" << string(50, ' ')
    << "Print version information and exit.\n"
    << "\n"
    << "Soft and/or hard limits may specified.  To set only the hard limit\n"
    << "prefix the value with a comma.  Limit values may be followed by 'k',\n"
    << "'m', or 'g' to multiply by 1,000, 1,000,000 or 1,000,000,000\n"
    << "respectively or by 'K', 'M', 'G' to multiply by 1,024, 1,048,576 or\n"
    << "1,073,741,824 respectively.  In other words, use upper case for SI\n"
    << "units.  If the hard limit is less less than than the soft limit the\n"
    << "soft limit will be lowered.\n"
    << "\n\n"
    << "If no limits are specified and no command is given then all limits\n"
    << "will be printed.  If a command is specified, any limits provided\n"
    << "will be set and the command will be run as a subprocess.\n"
    << flush;
}


void version() {
  cout
    << "rlimit 0.0.1\n"
    << "Copyright: (c) 2009 Cauldron Development LLC\n"
    << "   Author: Joseph Coffland <joseph@cauldrondevelopment.com>\n"
    << "  License: GNU General Public License version 2\n"
    << flush;
}


int main(int argc, char *argv[]) {
  try {
    bool haveLimit = false;

    // Count resources
    const unsigned count = NUM_RESOURCES;

    // Build options
    struct option options[count + 3];
    ostringstream optstr;
    optstr << '+';

    for (unsigned i = 0; i < count; i++) {
      // optstr
      optstr << resources[i].shortOpt << "::";

      // options
      options[i].name = resources[i].longOpt;
      options[i].has_arg = 2; // Optional
      options[i].flag = 0;
      options[i].val = resources[i].shortOpt;
    }

    // optstr
    optstr << "vhV" << ends;

    // options
    options[count + 0] = (struct option){"help", 2, 0, 1};
    options[count + 1] = (struct option){"version", 2, 0, 'V'};
    options[count + 2] = (struct option){0, 0, 0, 0};

    // Parse args
    while (true) {
      int i = 0;
      int c = getopt_long(argc, argv, optstr.str().c_str(), options, &i);
      if (c == -1) break;

      switch (c) {
      case 'v': verbosity++; break;
      case 'h': human = true; break;
      case 'V': version();  return 0;
      case 1: usage(cout, argv[0]); return 0; // --help
      default:
        Resource *resource = opt_to_resource(c);
        if (!resource) {usage(cerr, argv[0]); return 1;}
        haveLimit = true;
        resource->doLimit(optarg);
      }
    }

    if (!haveLimit && optind == argc) {
      // Print all limits
      for (unsigned i = 0; i < count; i++)
        resources[i].doLimit();

    } else if (optind < argc) {
      // Run command
      if (execvp(argv[optind], argv + optind)) {
        ostringstream str;
        for (int i = optind; i < argc; i++) {
          // TODO Escape single quotes in args with white space

          bool hasWS = false;
          for (int j = 0; argv[i][j]; j++)
            if (isspace(argv[i][j])) {hasWS = true; break;}

          if (hasWS) str << "'";
          str << " " << argv[i];
          if (hasWS) str << "'";
        }

        THROW("Executing: " << str.str());
      }
    }

    return 0;

  } catch (const std::exception &e) {
    cerr << "ERROR: " << e.what();
    if (errno) cerr << ": " << strerror(errno);
    cerr << endl;
  }

  return 1;
}
