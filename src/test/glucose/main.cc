#include <base/printf.h>
#include <timer_session/connection.h>

#include <glucose/core/Solver.h>
#include <glucose/core/Dimacs.h>

#include <trace/timestamp.h>

// this value is for the Raspberry Pi
#define TICKS_PER_USEC (700.0/64.0)

using namespace Glucose;

inline double cpuTime()
{
	static Timer::Connection _timer;
	return _timer.elapsed_ms();
}

void printStats(Solver& solver, double initial_time, Genode::Trace::Timestamp start_ts)
{
	 Genode::Trace::Timestamp ts_ticks = Genode::Trace::timestamp() - start_ts;
    double cpu_time = cpuTime() - initial_time;
    double mem_used = 0;//memUsedPeak();
    printf("c restarts              : %lld (%lld conflicts in avg)\n", solver.starts,(solver.starts>0 ?solver.conflicts/solver.starts : 0));
    printf("c blocked restarts      : %lld (multiple: %lld) \n", solver.nbstopsrestarts,solver.nbstopsrestartssame);
    printf("c last block at restart : %lld\n",solver.lastblockatrestart);
    printf("c nb ReduceDB           : %lld\n", solver.nbReduceDB);
    printf("c nb removed Clauses    : %lld\n",solver.nbRemovedClauses);
    printf("c nb learnts DL2        : %lld\n", solver.nbDL2);
    printf("c nb learnts size 2     : %lld\n", solver.nbBin);
    printf("c nb learnts size 1     : %lld\n", solver.nbUn);

    printf("c conflicts             : %-12lld   (%.0f /sec)\n", solver.conflicts   , solver.conflicts   /cpu_time);
    printf("c decisions             : %-12lld   (%4.2f %% random) (%.0f /sec)\n", solver.decisions, (float)solver.rnd_decisions*100 / (float)solver.decisions, solver.decisions   /cpu_time);
    printf("c propagations          : %-12lld   (%.0f /sec)\n", solver.propagations, solver.propagations/cpu_time);
    printf("c conflict literals     : %-12lld   (%4.2f %% deleted)\n", solver.tot_literals, (solver.max_literals - solver.tot_literals)*100 / (double)solver.max_literals);
    printf("c nb reduced Clauses    : %lld\n",solver.nbReducedClauses);
    
    if (mem_used != 0) printf("Memory used           : %.2f MB\n", mem_used);
    printf("c CPU time              : %.2f ms\n", cpu_time);
    printf("c TS ticks              : %d (%.1f µs)\n", ts_ticks, (double)ts_ticks/TICKS_PER_USEC);
}

int main()
{
	Solver S;

	gzFile gzfile_components = gzopen("Test.cnf", "rb");	
	if (gzfile_components != NULL) {
		double initial_time = cpuTime();

		Genode::Trace::Timestamp start_ts = Genode::Trace::timestamp();
//		Genode::Trace::Timestamp start_ts = 0;
		parse_DIMACS(gzfile_components, S);
		gzclose(gzfile_components);

		double parsed_time = cpuTime();
		Genode::Trace::Timestamp parse_ticks = Genode::Trace::timestamp();

		/* run solver */
		printf("c |  Number of variables:  %12d                                                                   |\n", S.nVars());
		printf("c |  Number of clauses:    %12d                                                                   |\n", S.nClauses());

		printf("c |  Parse time:           %12.2f ms                                                                 |\n", parsed_time - initial_time);
		printf("c |  Parse ticks:          %d (%.1f µs)                                                              |\n", parse_ticks - start_ts, (double)(parse_ticks-start_ts)/TICKS_PER_USEC);
		printf("c |                                                                                                       |\n");
 
		if (!S.simplify()){
			printf("c =========================================================================================================\n");
			printf("Solved by unit propagation\n");
			printStats(S, initial_time, start_ts);
			printf("\n");
			printf("s UNSATISFIABLE\n");
			exit(20);
		}

		vec<Lit> dummy;
		lbool ret = S.solveLimited(dummy);
		printStats(S, initial_time, start_ts);
		printf("\n");

		printf(ret == l_True ? "s SATISFIABLE\n" : ret == l_False ? "s UNSATISFIABLE\n" : "s INDETERMINATE\n");
		if(ret==l_True) {
			printf("v ");
			for (int i = 0; i < S.nVars(); i++)
				if (S.model[i] != l_Undef)

			printf("%s%s%d", (i==0)?"":" ", (S.model[i]==l_True)?"":"-", i+1);
			printf(" 0\n");
		}

		/* solver finished */

		PINF("Okay");
	}
	else {
		PERR("Unable to open file \"Test.cnf\"");
		return 1;
	}

	return 0;
}
