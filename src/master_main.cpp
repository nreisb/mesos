#include <getopt.h>

#include "master.hpp"
#include "master_webui.hpp"
#include "zookeeper_master.hpp"


using std::cerr;
using std::endl;
using namespace nexus::internal::master;


void usage(const char* programName)
{
  cerr << "Usage: " << programName
       << " [--port PORT]"
       << " [--allocator ALLOCATOR]"
       << " [--zookeeper host:port]"
       << " [--quiet]"
       << endl;
}


int main (int argc, char **argv)
{
  if (argc == 2 && string("--help") == argv[1]) {
    usage(argv[0]);
    exit(1);
  }

  option options[] = {
    {"allocator", required_argument, 0, 'a'},
    {"port", required_argument, 0, 'p'},
    {"zookeeper", required_argument, 0, 'z'},
    {"quiet", no_argument, 0, 'q'},
  };

  bool quiet = false;
  string allocator = "simple";
  string zookeeper;

  int opt;
  int index;
  while ((opt = getopt_long(argc, argv, "a:p:z:q", options, &index)) != -1) {
    switch (opt) {
      case 'a':
        allocator = optarg;
        break;
      case 'p':
        setenv("LIBPROCESS_PORT", optarg, 1);
        break;
      case 'z':
        zookeeper = optarg;
	break;
      case 'q':
        quiet = true;
        break;
      case '?':
        // Error parsing options; getopt prints an error message, so just exit
        exit(1);
        break;
      default:
        break;
    }
  }

  if (!quiet)
    google::SetStderrLogging(google::INFO);
  
  FLAGS_logbufsecs = 1;
  google::InitGoogleLogging(argv[0]);

  LOG(INFO) << "Build: " << BUILD_DATE << " by " << BUILD_USER;
  LOG(INFO) << "Starting Nexus master";
  Master* master = new Master(allocator);
  PID pid = Process::spawn(master);

  if (!zookeeper.empty())
    Process::spawn(new ZooKeeperProcessForMaster(pid, zookeeper));

#ifdef NEXUS_WEBUI
  startMasterWebUI(pid);
#endif
  
  Process::wait(pid);
  return 0;
}
