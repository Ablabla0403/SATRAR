cirGate.o: cirGate.cpp cirGate.h cirDef.h cirMgr.h ../sat/sat.h \
 ../sat/Solver.h ../sat/SolverTypes.h ../sat/Global.h ../sat/VarOrder.h \
 ../sat/Heap.h ../sat/Proof.h ../sat/File.h ../../include/util.h \
 ../../include/rnGen.h ../../include/myUsage.h
cirMgr.o: cirMgr.cpp cirMgr.h cirDef.h ../sat/sat.h ../sat/Solver.h \
 ../sat/SolverTypes.h ../sat/Global.h ../sat/VarOrder.h ../sat/Heap.h \
 ../sat/Proof.h ../sat/File.h cirGate.h ../../include/util.h \
 ../../include/rnGen.h ../../include/myUsage.h
cirCmd.o: cirCmd.cpp cirMgr.h cirDef.h ../sat/sat.h ../sat/Solver.h \
 ../sat/SolverTypes.h ../sat/Global.h ../sat/VarOrder.h ../sat/Heap.h \
 ../sat/Proof.h ../sat/File.h cirGate.h cirCmd.h \
 ../../include/cmdParser.h ../../include/cmdCharDef.h \
 ../../include/util.h ../../include/rnGen.h ../../include/myUsage.h
