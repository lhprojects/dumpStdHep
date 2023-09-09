

#include <StdHepIter.h>
#include <EVENT/LCEvent.h>
#include <IO/ILCFactory.h>
//#include <SIO/SIOReader.h>

#include <stdio.h>
#include <map>
#include <exception>
#include <math.h>
#include <string.h>
#include <string>
#include <vector>

#include "PrintCollection.h"

using namespace std;
using namespace EVENT;
using namespace lcio;

bool ends_with(const std::string& str, const std::string& sub) {
    return sub.length() <= str.length()
        && equal(sub.begin(), sub.end(), str.end() - sub.length());
}



void print_help() {
    printf("dumpStdHep [options] <file> <run event_number | nth (start with 0)>\n");
    printf(".slcio (MCParticle collection used) and .stdhep file format supported\n");
    printf("print final states for (run,event_number) or nth event in <file>\n");
    printf("(run,event_number) not supported for .stdhep file\n");
    printf("    --help, -h     :   print help\n");
    printf("    --tree, -t     :   tree style output\n");
    printf("    --vertex, -vtx :   show vertex position\n");
    printf("    --endpoint, -end : show endpoint position\n");
    printf("    --generator, -g:   show particles only with generator status not empty\n");
    printf("    --pdgid, -id   :   show pdgid\n");
    printf("    --generator_status, -gs   :   show generator status\n");
    printf("    --simulator_status, -ss   :   show simulator status\n");
    printf("    --created_in_simulation, -cis   :   show if particle is created in simulation\n");
}


void read_options(int argc, char *argv[], Options &opt) {

    if(argc < 3) {
        print_help();
        exit(1);
    }

    for(int i = 1; i < argc; ++i) {
    
        if(argv[i][0] == '-'){
            if(strcmp("--tree",argv[i]) == 0 || strcmp("-t", argv[i]) == 0) {
                opt.tree = true;
            } else if(strcmp("--help",argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
                opt.help = true;
                print_help();
            } else if(strcmp("--vertex",argv[i]) == 0 || strcmp("-vtx", argv[i]) == 0) {
                opt.vertex = true;
            } else if(strcmp("--endpoint",argv[i]) == 0 || strcmp("-end", argv[i]) == 0) {
                opt.endpoint = true;
            } else if(strcmp("--pdgid",argv[i]) == 0 || strcmp("-id", argv[i]) == 0) {
                opt.pdgid = true;
            } else if(strcmp("--generator",argv[i]) == 0 || strcmp("-g", argv[i]) == 0) {
                opt.generator = true;
            } else if(strcmp("--generator_status",argv[i]) == 0 || strcmp("-gs", argv[i]) == 0) {
                opt.generator_status = true;
            } else if(strcmp("--simulator_status",argv[i]) == 0 || strcmp("-ss", argv[i]) == 0) {
                opt.simulator_status = true;
            } else if(strcmp("--created_in_simulation",argv[i]) == 0 || strcmp("-cis", argv[i]) == 0) {
                opt.created_in_simulation = true;
            }  else {
                fprintf(stderr, "unkown option %s\n", argv[i]);
                exit(1);
            }
        } else if (!opt.file) {
            opt.file = argv[i];
        } else if (opt.run < 0) {
            char const *n_str = argv[i];
            if (sscanf(n_str, "%d", &opt.run) < 1 || opt.run < 0)
            {
                fprintf(stderr, "A number expected: %s\n", n_str);
                exit(1);
            }
        } else if(opt.n < 0) {
            char const *n_str = argv[i];
            if (sscanf(n_str, "%d", &opt.n) < 1 || opt.n < 0)
            {
                fprintf(stderr, "A number expected: %s\n", n_str);
                exit(1);
            }
        } else {
            fprintf(stderr, "what is this `%s`?\n", argv[i]);
            exit(1);
        }    
    }
    
    if(!opt.file) {
        fprintf(stderr, "no file specified\n");
        exit(1);
    }

    if(opt.n < 0 && opt.run < 0) {
        fprintf(stderr, "no event number specified\n");
        exit(1);
    }
    if(opt.run >= 0 && opt.n < 0) {
        opt.n = opt.run;
        opt.run = -1;
    }




}


void read_file(Options const &opt) {

    char const *file = opt.file;
    int run = opt.run;
    int n = opt.n;

    if(ends_with(string(file), string(".stdhep"))) {

        if(run > 0) {
            printf("run number %d not support for stdhep\n", run);
            exit(1);
        }
        try {
            LCStdHepRdr rdr(file);

            LCCollection *col;
            // skip until nth event
            int nevts = rdr.getNumberOfEvents();
            if(n >= nevts) {
                fprintf(stderr, "%d out of number of events %d in file\n", n, nevts);
                exit(1);
            }

            for(int i = 0; (col = rdr.readEvent()); ++i) {
                if(i == n) {
                    break;
                }
            }
            if(col) {
                printCollection(col, opt);
                delete col;
            }

        } catch(...) {
            printf("Exception for file %s\n", file);
        }

    } else if(ends_with(string(file), string(".slcio"))) {

        try {

            EVENT::LCEvent * evt = NULL;
            if(run >= 0) {
                LCReader &rdr = *(LCFactory::getInstance()->createLCReader(IO::LCReader::directAccess));
                rdr.open(file);
                evt = rdr.readEvent(run, n);
                if(!evt) {
                    fprintf(stderr, "run:%d no.%d not found\n", run, n);
                    exit(1);
                }
            } else {
                LCReader &rdr = *(LCFactory::getInstance()->createLCReader());
                rdr.open(file);
                int nevts = rdr.getNumberOfEvents();
                if(n >= nevts) {
                    fprintf(stderr, "%d out of number of events %d in file\n", n, nevts);
                    exit(1);
                }
                for(int i = 0; (evt = rdr.readNextEvent()); ++i) {
                    if(i == n) {
                        break;
                    }
                }
                if(!evt) {
                    fprintf(stderr, "%dth event ot found\n", n);
                    exit(1);
                }
            }
            if(evt) {
                LCCollection *col = evt->getCollection("MCParticle");
                if(!col) {
                    fprintf(stderr, "MCParticle collection not found\n");
                    exit(1);
                }
                printCollection(col, opt);
            }
            // memory leakage ...
        } catch(...) {
            fprintf(stderr, "Exception for file %s\n", file);
            exit(1);
        }
        
    } else {
        fprintf(stderr, "Exception for file %s\n", file);
            exit(1);
    }

}

int main(int argc, char *argv[]) {

    Options opt;
    read_options(argc, argv, opt);
    read_file(opt); 
    return 0;
}
