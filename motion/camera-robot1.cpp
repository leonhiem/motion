#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <errno.h>
#include <syslog.h>

using namespace std;

#define ONE_SECOND 1000000
main(int argc, char* argv[])
{
  while(1) {
      ostringstream cmd;

      // Usage: ./motion myname hostname port fuzzfactor threshold disable_extra

      cmd << "~/motion/motion robot1 10.0.0.12 3003 30 50 1";

      cout << "Starting up: " << cmd.str() << endl;
      system(cmd.str().c_str());
      usleep(10*ONE_SECOND);
  }
}

