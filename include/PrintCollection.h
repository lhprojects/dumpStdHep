#ifndef PRINT_COLLECTION
#define PRINT_COLLECTION


#include "IMPL/LCEventImpl.h"
#include "IMPL/MCParticleImpl.h"
#include "IMPL/LCRunHeaderImpl.h"
#include "UTIL/LCStdHepRdr.h"
#include "UTIL/LCTOOLS.h"
#include "marlin/DataSourceProcessor.h"
#include "marlin/ProcessorMgr.h"



#include <stdio.h>
#include <map>
#include <exception>
#include <math.h>
#include <string.h>
#include <string>
#include <vector>



// options
struct Options {
    bool vertex = false;
    bool endpoint = false;
    bool generator = false;
    bool help = false;
    bool pdgid = false;
    bool tree = false;
    char const *file = nullptr;
    int run = -1;
    int n = -1;
    bool simulator_status = false;
    bool generator_status = false;
    bool created_in_simulation = false;
};



inline char const *genStatus(int gen) {
    if(gen == 0) return "empty";
    else if(gen == 1) return "stable";
    else if(gen == 2) return "decayed";
    else if(gen == 3) return "doc";
    else {
        static char b[100];
        sprintf(b, "%d", gen);
        return b;
    }
}

inline char const*name(int pdg, Options const &opt) {


    char const *name = "";
    static const std::map<int, char const *> pdgToName =  
#include "map.h" 
    ;
    auto iter = pdgToName.find(pdg);
    if(iter != pdgToName.end())
        name =  iter->second;
    
    if(opt.pdgid || strlen(name) == 0) {
        static char b[100];
        sprintf(b, "%s(%d)", name, pdg);
        return b;
    } else {
        return name;
    }

}

inline double SignedMass(double e, double const *p) {

    double e2 = e*e;
    double p2 = p[0]*p[0] + p[1]*p[1] + p[2]*p[2];
    double m2 = e2 - p2;
    if(m2>0) return sqrt(m2);
    else return -sqrt(-m2);
}


struct PrintCollection {
    
        
    int line = 0;
    bool headerPrint;
    std::map<MCParticle *, bool> printed;
    std::map<MCParticle *, bool> inCollection;
    std::map<MCParticle *, int> lines;
    std::vector<int> draw_splits;



void printTree(MCParticle *mcp, bool toplevel, Options const &opt)
{

   

    if(!headerPrint)
        printf("%4s %8s %2s %2s %8s %8s %8s %8s %8s\n", "Line", "Status", "#P", "#D", "M", "E", "Px", "Py", "Pz");
    headerPrint = true;
    
    
    if (!printed[mcp])
    {
        printed[mcp] = true;
        lines[mcp] = line;
        //printf("%d\n", line);
        double const *p = mcp->getMomentum();       
        printf("%4d %8s %2d %2d %8.3f %8.3f %8.3f %8.3f %8.3f ",
               line,
               genStatus(mcp->getGeneratorStatus()),
               (int)mcp->getParents().size(),
               (int)mcp->getDaughters().size(),
               SignedMass(mcp->getEnergy(),p),
               mcp->getEnergy(), p[0], p[1], p[2]);
        ++line;
        
        for(int i = 0; i < (int)draw_splits.size(); ++i) {

            if(i == (int)draw_splits.size() - 1) { // override the spliter by particle name
                if(draw_splits.at(i))
                    printf("|- ");                 // this particle is not the last daughter
                else
                    printf("\\- ");                // this particle is the last daughter
            } else {
 
                if(draw_splits.at(i)) {
                    printf("|  ");
                } else {
                    printf("   ");
                }           
            
            }

        };
        
        
        printf("%-s", name(mcp->getPDG(), opt));

        if (opt.vertex)
            printf("[%g, %g, %g]", mcp->getVertex()[0], mcp->getVertex()[1], mcp->getVertex()[2]);
        if (opt.endpoint)
            printf("(%g, %g, %g)", mcp->getEndpoint()[0], mcp->getEndpoint()[1], mcp->getEndpoint()[2]);

        if (opt.simulator_status) {
            printf(" ss%d", (int)mcp->getSimulatorStatus());
        }
        if (opt.generator_status) {
            printf(" gs%d", (int)mcp->getGeneratorStatus());
        }
        if (opt.created_in_simulation) {
            printf(" cis%d", (int)mcp->isCreatedInSimulation());
        }

        //if(mcp->getPDG()) {
            //printf(" ic%d", inCollection[mcp]); 
        //}
 

        printf("\n");

        for (int i = 0; i < (int)mcp->getDaughters().size(); ++i)
        {
            MCParticle* daug = (MCParticle*)(mcp->getDaughters()[i]);
            if(opt.generator && daug->getGeneratorStatus() == 0) continue;
            if(!inCollection[daug]) continue;
            
            
            // we are the last daughter? let the space black
            // we are not the last daughter? we print a spliter `|  '
            // |- gamma <- not black, but override by particle name
            // | <- not black
            // \- Higgs <- black, but override by particle name
            //   <- black
            //   <- black
            draw_splits.push_back(i + 1 !=  (int)mcp->getDaughters().size());
            printTree(daug, false, opt);
            draw_splits.pop_back();
        }

    } else {
        // toplevel particles? print nothing if we has printed a line for it
        if(!toplevel) {
            printf("%4s %8s %2s %2s %8s %8s %8s %8s %8s ", "", "", "", "", "", "", "", "", "");

    
    
        for(int i = 0; i < (int)draw_splits.size(); ++i) {

            if(i == (int)draw_splits.size() - 1) {
                if(draw_splits.at(i))
                    printf("|- ");
                else
                    printf("\\- ");
                continue;
            }

            if(draw_splits.at(i)) {
                printf("|  ");
            } else {
                printf("   ");
            }
         }
        
 
 
        printf("L. %d\n", lines[mcp]);
    
        
        } // if (!toplevel)
    }
}



    void print(LCCollection *col, Options const &opt) {
    
    if (!opt.tree)
    {
        printf("Idx %8s %8s %2s %8s %8s %8s %8s\n", "Partcle", "Status", "Ps", "E", "Px", "Py", "Pz");
        for (int i = 0; i < col->getNumberOfElements(); ++i)
        {
            MCParticle *mcp = (MCParticle *)col->getElementAt(i);
            if(opt.generator && mcp->getGeneratorStatus() == 0) continue;
            double const *p = mcp->getMomentum();
            printf("%3d %8s %8s %2d %8.3f %8.3f %8.3f %8.3f\n",
                    i, name(mcp->getPDG(), opt), genStatus(mcp->getGeneratorStatus()),
                    (int)mcp->getParents().size(),
                    mcp->getEnergy(), p[0], p[1], p[2]);
        }
    } else {
        for (int i = 0; i < col->getNumberOfElements(); ++i)
        {
            MCParticle *mcp = (MCParticle *)col->getElementAt(i);
            inCollection[mcp] = true;
        }
 

        for (int i = 0; i < col->getNumberOfElements(); ++i)
        {
            MCParticle *mcp = (MCParticle *)col->getElementAt(i);
            if(opt.generator && mcp->getGeneratorStatus() == 0)
                continue;
            printTree(mcp, true, opt);
        }
    }


    
    
    
    
    }



};


inline void printCollection(LCCollection *col, Options const &opt) {
    PrintCollection printCollection;
    printCollection.print(col, opt);
}


#endif

