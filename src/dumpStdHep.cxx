
#include <stdio.h>

#include <map>

#include <StdHepIter.h>
#include <TLorentzVector.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TRandom.h>
#include <TH2D.h>
#include <exception>
using namespace EVENT;
using namespace lcio;


char const *genStatus(int gen) {
    if(gen == 0) return "empty";
    else if(gen == 1) return "stable";
    else if(gen == 2) return "decayed";
    else if(gen == 3) return "doc";
    else return "gen";
}
char const*name(int pdg) {

        if(pdg == 25) return "H";
        else if(pdg == 1) return "d";
        else if(pdg == 2) return "u";
        else if(pdg == 3) return "s";
        else if(pdg == 4) return "c";
        else if(pdg == 5) return "b";
        else if(pdg == -5) return "Anti-b";
        else if(pdg == 22) return "gamma";
        else if(pdg == 11) return "e-";
        else if(pdg == -11) return "e+";
        else if(pdg == 12) return "ve";
        else if(pdg == -12) return "Anti-ve";
        else if(pdg == 13) return "mu-";
        else if(pdg == -13) return "mu+";
        else if(pdg == 14) return "vmu";
        else if(pdg == -14) return "Anti-vmu";
        else if(pdg == 15) return "tau-";
        else if(pdg == -15) return "tau+";
        else if(pdg == 16) return "vtau";
        else if(pdg == -16) return "Anti-vtau";
        else if(pdg == 21) return "g";
        else if(pdg == 111) return "pi0";
        else if(pdg == 211) return "pi+";
        else if(pdg == -211) return "pi-";
        else if(pdg == 113) return "rho0(770)";
        else if(pdg == 213) return "rho+(770)";
        else if(pdg == -213) return "rho-(770)";
        else if(pdg == 221) return "eta";
        else if(pdg == 223) return "omega(782)";
        else if(pdg == 331) return "eta^prime(958)";
        else if(pdg == 333) return "phi(1020)";
        else if(pdg == 313) return "K^star0(892)";
        else if(pdg == -313) return "Anti-K^star0(892)";
        else if(pdg == 323) return "K^star+(892)";
        else if(pdg == -323) return "K^star-(892)";
        else if(pdg == 130) return "KL0";
        else if(pdg == 310) return "KS0";
        else if(pdg == 311) return "K0";
        else if(pdg == 321) return "K+";
        else {
            static char b[100];
            sprintf(b, "%d", pdg);
            return b;
        }

}

double SignedMass(double e, double const *p) {

    double e2 = e*e;
    double p2 = p[0]*p[0] + p[1]*p[1] + p[2]*p[2];
    double m2 = e2 - p2;
    if(m2>0) return sqrt(m2);
    else return -sqrt(-m2);
}

void printTree(MCParticle *mcp, std::map<MCParticle *, bool> &printed, int space)
{

    if (!printed[mcp])
    {
        printed[mcp] = true;
        double const *p = mcp->getMomentum();       
        printf("%8s %3d %8.3f %8.3f %8.3f %8.3f %8.3f   ",
               genStatus(mcp->getGeneratorStatus()),
               (int)mcp->getParents().size(),
               SignedMass(mcp->getEnergy(),p),
               mcp->getEnergy(), p[0], p[1], p[2]);

        for(int i = 0; i < space; ++i) { if(i%3 == 0) printf("|"); else printf(" "); };
        printf("%-s[%g, %g, %g]\n",
               name(mcp->getPDG()), mcp->getVertex()[0], mcp->getVertex()[1], mcp->getVertex()[2]);

        for (int i = 0; i < (int)mcp->getDaughters().size(); ++i)
        {
            printTree((MCParticle*)(mcp->getDaughters()[i]), printed, space + 3);
        }
    }
}

int main(int argc, char *argv[]) {

    if(argc < 3) {
        printf("dumpStdHep [options] <file> <event_number(start with 0)>\n");
        printf("    --tree, -t :   tree style output\n");
        exit(1);
    }

    bool tree = false;
    char const *file = NULL;
    int n = -1;

    for(int i = 1; i < argc; ++i) {
    
        if(strcmp("--tree",argv[i]) == 0 || strcmp("-t", argv[i]) == 0) {
            tree = true;
        }
        else if (!file)
        {
            file = argv[i];
        }
        else if(n < 0)
        {
            char const *n_str = argv[i];
            if (sscanf(n_str, "%d", &n) < 1 || n < 0)
            {
                printf("event_number expected: %s\n", n_str);
                exit(1);
            }
        } else {
            printf("what is this `%s`?\n", argv[i]);
            exit(1);
        }
   
    
    }

    LCStdHepRdr rdr(file);
    LCCollection *col;
    for(int i = 0; (col = rdr.readEvent()); ++i) {
        if(i == n) {
            break;
        }
    }
    if(col) {
        if (!tree)
        {
            printf("Idx %8s %8s %2s %8s %8s %8s %8s\n", "Partcle", "Status", "Ps", "E", "Px", "Py", "Pz");
            for (int i = 0; i < col->getNumberOfElements(); ++i)
            {
                MCParticle *mcp = (MCParticle *)col->getElementAt(i);
                double const *p = mcp->getMomentum();
                printf("%3d %8s %8s %2d %8.3f %8.3f %8.3f %8.3f\n",
                       i, name(mcp->getPDG()), genStatus(mcp->getGeneratorStatus()),
                       (int)mcp->getParents().size(),
                       mcp->getEnergy(), p[0], p[1], p[2]);
            }
        } else {

            printf("%8s %3s %8s %8s %8s %8s %8s\n", "Status", "Par", "M", "E", "Px", "Py", "Pz");
            std::map<MCParticle *,bool> printed;
            for (int i = 0; i < col->getNumberOfElements(); ++i)
            {
                MCParticle *mcp = (MCParticle *)col->getElementAt(i);
                printTree(mcp, printed, 0);
            }

        }
    }

    return 0;
}
