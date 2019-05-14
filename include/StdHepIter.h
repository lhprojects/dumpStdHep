#ifndef STDHEPITER_H
#define STDHEPITER_H

#include "IMPL/LCEventImpl.h"
#include "IMPL/MCParticleImpl.h"
#include "IMPL/LCRunHeaderImpl.h"
#include "UTIL/LCStdHepRdr.h"
#include "UTIL/LCTOOLS.h"
#include "marlin/DataSourceProcessor.h"
#include "marlin/ProcessorMgr.h"

struct StdHepIter {
        StdHepIter(std::vector<std::string> const &files) {
            fFiles = files;
            fFile = 0;
            fRdr = NULL;
        }
        std::vector<std::string> fFiles;
        int fFile;
        lcio::LCStdHepRdr *fRdr;
        EVENT::LCCollection *getEvent() {
            using namespace EVENT;
            for(;;) {
                if(fRdr == NULL) {
                    if(fFile+1 >= (int)fFiles.size()) { // have one file left
                        return NULL;
                    }
                    fFile += 1;
                    fRdr = new lcio::LCStdHepRdr(fFiles[fFile].c_str());
                } else {
                    LCCollection * col = fRdr->readEvent();
                    if(col) return col; 
                    delete fRdr;
                    fRdr = NULL;
                }
            
            }
       }

       ~StdHepIter() {
            if(fRdr) delete fRdr;
       }
};


#endif



