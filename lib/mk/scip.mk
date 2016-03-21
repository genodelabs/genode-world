SCIP_DIR = $(call select_from_ports,scip)/src/lib/scip/src
LIBS    += libc zlib 
INC_DIR += $(SCIP_DIR)

# plugin files
SRC_C    =       scip/branch_allfullstrong.c \
			scip/branch_cloud.c \
			scip/branch_fullstrong.c \
			scip/branch_inference.c \
			scip/branch_leastinf.c \
			scip/branch_mostinf.c \
			scip/branch_pscost.c \
			scip/branch_random.c \
			scip/branch_relpscost.c \
			scip/cons_abspower.c \
			scip/cons_and.c \
			scip/cons_bivariate.c \
			scip/cons_bounddisjunction.c \
			scip/cons_conjunction.c \
			scip/cons_countsols.c \
			scip/cons_cumulative.c \
			scip/cons_disjunction.c \
			scip/cons_indicator.c \
			scip/cons_integral.c \
			scip/cons_knapsack.c \
			scip/cons_linear.c \
			scip/cons_linking.c \
			scip/cons_logicor.c \
			scip/cons_nonlinear.c \
			scip/cons_or.c \
			scip/cons_orbitope.c \
			scip/cons_pseudoboolean.c \
			scip/cons_quadratic.c \
			scip/cons_setppc.c \
			scip/cons_soc.c \
			scip/cons_sos1.c \
			scip/cons_sos2.c \
			scip/cons_superindicator.c \
			scip/cons_varbound.c \
			scip/cons_xor.c \
			scip/dialog_default.c \
			scip/disp_default.c \
			scip/heur_actconsdiving.c \
			scip/heur_clique.c \
			scip/heur_coefdiving.c \
			scip/heur_crossover.c \
			scip/heur_dins.c \
			scip/heur_dualval.c \
			scip/heur_feaspump.c \
			scip/heur_fixandinfer.c \
			scip/heur_fracdiving.c \
			scip/heur_guideddiving.c \
			scip/heur_zeroobj.c \
			scip/heur_intdiving.c \
			scip/heur_intshifting.c \
			scip/heur_linesearchdiving.c \
			scip/heur_localbranching.c \
			scip/heur_mutation.c \
			scip/heur_nlpdiving.c \
			scip/heur_objpscostdiving.c \
			scip/heur_octane.c \
			scip/heur_oneopt.c \
			scip/heur_proximity.c \
			scip/heur_pscostdiving.c \
			scip/heur_rens.c \
			scip/heur_randrounding.c \
			scip/heur_rins.c \
			scip/heur_rootsoldiving.c \
			scip/heur_rounding.c \
			scip/heur_shiftandpropagate.c \
			scip/heur_shifting.c \
			scip/heur_simplerounding.c \
			scip/heur_subnlp.c \
			scip/heur_trivial.c \
			scip/heur_trysol.c \
			scip/heur_twoopt.c \
			scip/heur_undercover.c \
			scip/heur_vbounds.c \
			scip/heur_veclendiving.c \
			scip/heur_zirounding.c \
			scip/message_default.c \
			scip/nodesel_bfs.c \
			scip/nodesel_breadthfirst.c \
			scip/nodesel_dfs.c \
			scip/nodesel_estimate.c \
			scip/nodesel_hybridestim.c \
			scip/nodesel_restartdfs.c \
			scip/nodesel_uct.c \
			scip/presol_boundshift.c \
			scip/presol_components.c \
			scip/presol_convertinttobin.c \
			scip/presol_domcol.c\
			scip/presol_dualinfer.c\
			scip/presol_gateextraction.c \
			scip/presol_implics.c \
			scip/presol_inttobinary.c \
			scip/presol_trivial.c \
			scip/prop_dualfix.c \
			scip/prop_genvbounds.c \
			scip/prop_obbt.c \
			scip/prop_probing.c \
			scip/prop_pseudoobj.c \
			scip/prop_redcost.c \
			scip/prop_rootredcost.c \
			scip/prop_vbounds.c \
			scip/reader_bnd.c \
			scip/reader_ccg.c \
			scip/reader_cip.c \
			scip/reader_cnf.c \
			scip/reader_fix.c \
			scip/reader_fzn.c \
			scip/reader_gms.c \
			scip/reader_lp.c \
			scip/reader_mps.c \
			scip/reader_opb.c \
			scip/reader_osil.c \
			scip/reader_pip.c \
			scip/reader_ppm.c \
			scip/reader_pbm.c \
			scip/reader_rlp.c \
			scip/reader_sol.c \
			scip/reader_wbo.c \
			scip/reader_zpl.c \
			scip/sepa_cgmip.c \
			scip/sepa_clique.c \
			scip/sepa_closecuts.c \
			scip/sepa_cmir.c \
			scip/sepa_flowcover.c \
			scip/sepa_gomory.c \
			scip/sepa_impliedbounds.c \
			scip/sepa_intobj.c \
			scip/sepa_mcf.c \
			scip/sepa_oddcycle.c \
			scip/sepa_rapidlearning.c \
			scip/sepa_strongcg.c \
			scip/sepa_zerohalf.c

# library files
SRC_C   +=	scip/branch.c \
			scip/buffer.c \
			scip/clock.c \
			scip/conflict.c \
			scip/cons.c \
			scip/cutpool.c \
			scip/debug.c \
			scip/dialog.c \
			scip/disp.c \
			scip/event.c \
			scip/fileio.c \
			scip/heur.c \
			scip/history.c \
			scip/implics.c \
			scip/interrupt.c \
			scip/intervalarith.c \
			scip/lp.c \
			scip/mem.c \
			scip/misc.c \
			scip/nlp.c \
			scip/nodesel.c \
			scip/paramset.c \
			scip/presol.c \
			scip/presolve.c \
			scip/pricestore.c \
			scip/pricer.c \
			scip/primal.c \
			scip/prob.c \
			scip/prop.c \
			scip/reader.c \
			scip/relax.c \
			scip/retcode.c \
			scip/scip.c \
			scip/scipdefplugins.c \
			scip/scipgithash.c \
			scip/scipshell.c \
			scip/sepa.c \
			scip/sepastore.c \
			scip/set.c \
			scip/sol.c \
			scip/solve.c \
			scip/stat.c \
			scip/tree.c \
			scip/var.c \
			scip/vbc.c \
			tclique/tclique_branch.c \
			tclique/tclique_coloring.c \
			tclique/tclique_graph.c \
			dijkstra/dijkstra.c \
			xml/xmlparse.c

# LP interface (soplex)
SOPLEX_DIR = $(call select_from_ports,soplex)/src/lib/soplex/src
INC_DIR += $(SOPLEX_DIR)
LIBS    += stdcxx soplex
SRC_CC  +=	lpi/lpi_spx2.cpp
SRC_C   +=	scip/bitencode.c \
			blockmemshell/memory.c \
			scip/message.c

# NLP Interface
SRC_C   += nlpi/nlpi.c \
			nlpi/nlpioracle.c \
			nlpi/expr.c \
			scip/misc.c \
			scip/intervalarith.c \
			scip/interrupt.c \
			scip/message.c \
			blockmemshell/memory.c \
			nlpi/exprinterpret_none.c \
			nlpi/nlpi_ipopt_dummy.c

vpath %.c   $(SCIP_DIR)
vpath %.cpp $(SCIP_DIR)

SHARED_LIB = yes
