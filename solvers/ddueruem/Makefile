BUDDY_SRC 	= .cache/buddy-2.4/src
CUDD_SRC 	= .cache/cudd-3.0.0

CC 			= gcc
CFLAGS 		= -fPIC -mtune=native -DHAVE_IEEE_754 -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8 -std=c99 -O3

INCLUDES_CUDD 	= 	-I/usr/include \
					-I$(CUDD_SRC) -I$(CUDD_SRC)/cudd -I$(CUDD_SRC)/epd -I$(CUDD_SRC)/mtr -I$(CUDD_SRC)/st -I$(CUDD_SRC)/util

INCLUDES_BUDDY	= 	-I$(BUDDY_SRC) -I$(BUDDY_SRC)/..

BUDDY_DLL_NAME 	= libbuddy.so
BUDDY_SRCS = $(BUDDY_SRC)/bddio.c $(BUDDY_SRC)/bddop.c $(BUDDY_SRC)/bvec.c \
	$(BUDDY_SRC)/cache.c $(BUDDY_SRC)/fdd.c $(BUDDY_SRC)/imatrix.c \
	$(BUDDY_SRC)/kernel.c $(BUDDY_SRC)/pairs.c $(BUDDY_SRC)/prime.c \
	$(BUDDY_SRC)/reorder.c $(BUDDY_SRC)/tree.c

BUDDY_OBJS = $(BUDDY_SRCS:.c=.o)

CUDD_DLL_NAME	= libcudd.so
CUDD_SRCS = $(CUDD_SRC)/cudd/cuddAPI.c $(CUDD_SRC)/cudd/cuddAddAbs.c $(CUDD_SRC)/cudd/cuddAddApply.c $(CUDD_SRC)/cudd/cuddAddFind.c \
	$(CUDD_SRC)/cudd/cuddAddInv.c $(CUDD_SRC)/cudd/cuddAddIte.c $(CUDD_SRC)/cudd/cuddAddNeg.c \
	$(CUDD_SRC)/cudd/cuddAddWalsh.c $(CUDD_SRC)/cudd/cuddAndAbs.c $(CUDD_SRC)/cudd/cuddAnneal.c \
	$(CUDD_SRC)/cudd/cuddApa.c  $(CUDD_SRC)/cudd/cuddApprox.c \
	$(CUDD_SRC)/cudd/cuddBddAbs.c $(CUDD_SRC)/cudd/cuddBddCorr.c $(CUDD_SRC)/cudd/cuddBddIte.c \
	$(CUDD_SRC)/cudd/cuddBridge.c $(CUDD_SRC)/cudd/cuddCache.c $(CUDD_SRC)/cudd/cuddCheck.c \
	$(CUDD_SRC)/cudd/cuddClip.c $(CUDD_SRC)/cudd/cuddCof.c $(CUDD_SRC)/cudd/cuddCompose.c \
	$(CUDD_SRC)/cudd/cuddDecomp.c $(CUDD_SRC)/cudd/cuddEssent.c $(CUDD_SRC)/cudd/cuddExact.c \
	$(CUDD_SRC)/cudd/cuddExport.c $(CUDD_SRC)/cudd/cuddGenCof.c $(CUDD_SRC)/cudd/cuddGenetic.c \
	$(CUDD_SRC)/cudd/cuddGroup.c $(CUDD_SRC)/cudd/cuddHarwell.c $(CUDD_SRC)/cudd/cuddInit.c \
	$(CUDD_SRC)/cudd/cuddInteract.c $(CUDD_SRC)/cudd/cuddLCache.c $(CUDD_SRC)/cudd/cuddLevelQ.c \
	$(CUDD_SRC)/cudd/cuddLinear.c $(CUDD_SRC)/cudd/cuddLiteral.c $(CUDD_SRC)/cudd/cuddMatMult.c \
	$(CUDD_SRC)/cudd/cuddPriority.c $(CUDD_SRC)/cudd/cuddRead.c $(CUDD_SRC)/cudd/cuddRef.c \
	$(CUDD_SRC)/cudd/cuddReorder.c $(CUDD_SRC)/cudd/cuddSat.c $(CUDD_SRC)/cudd/cuddSign.c \
	$(CUDD_SRC)/cudd/cuddSolve.c $(CUDD_SRC)/cudd/cuddSplit.c $(CUDD_SRC)/cudd/cuddSubsetHB.c \
	$(CUDD_SRC)/cudd/cuddSubsetSP.c $(CUDD_SRC)/cudd/cuddSymmetry.c $(CUDD_SRC)/cudd/cuddTable.c \
	$(CUDD_SRC)/cudd/cuddUtil.c $(CUDD_SRC)/cudd/cuddWindow.c $(CUDD_SRC)/cudd/cuddZddCount.c \
	$(CUDD_SRC)/cudd/cuddZddFuncs.c $(CUDD_SRC)/cudd/cuddZddGroup.c $(CUDD_SRC)/cudd/cuddZddIsop.c \
	$(CUDD_SRC)/cudd/cuddZddLin.c $(CUDD_SRC)/cudd/cuddZddMisc.c $(CUDD_SRC)/cudd/cuddZddPort.c \
	$(CUDD_SRC)/cudd/cuddZddReord.c $(CUDD_SRC)/cudd/cuddZddSetop.c $(CUDD_SRC)/cudd/cuddZddSymm.c \
	$(CUDD_SRC)/cudd/cuddZddUtil.c \
	$(CUDD_SRC)/util/cpu_stats.c $(CUDD_SRC)/util/cpu_time.c $(CUDD_SRC)/util/cstringstream.c \
	$(CUDD_SRC)/util/datalimit.c $(CUDD_SRC)/util/pathsearch.c $(CUDD_SRC)/util/pipefork.c \
	$(CUDD_SRC)/util/prtime.c $(CUDD_SRC)/util/safe_mem.c $(CUDD_SRC)/util/strsav.c $(CUDD_SRC)/util/texpand.c \
	$(CUDD_SRC)/util/ucbqsort.c $(CUDD_SRC)/st/st.c $(CUDD_SRC)/epd/epd.c $(CUDD_SRC)/mtr/mtrBasic.c $(CUDD_SRC)/mtr/mtrGroup.c \
	$(CUDD_SRC)/dddmp/dddmpBinary.c \
	$(CUDD_SRC)/dddmp/dddmpConvert.c $(CUDD_SRC)/dddmp/dddmpDbg.c $(CUDD_SRC)/dddmp/dddmpLoad.c \
	$(CUDD_SRC)/dddmp/dddmpLoadCnf.c $(CUDD_SRC)/dddmp/dddmpNodeAdd.c $(CUDD_SRC)/dddmp/dddmpNodeBdd.c \
	$(CUDD_SRC)/dddmp/dddmpNodeCnf.c $(CUDD_SRC)/dddmp/dddmpStoreAdd.c \
	$(CUDD_SRC)/dddmp/dddmpStoreBdd.c $(CUDD_SRC)/dddmp/dddmpStoreCnf.c \
	$(CUDD_SRC)/dddmp/dddmpStoreMisc.c $(CUDD_SRC)/dddmp/dddmpUtil.c

CUDD_OBJS = $(CUDD_SRCS:.c=.o)

default: buddy cudd

buddy: $(BUDDY_DLL_NAME)

$(BUDDY_DLL_NAME): $(BUDDY_OBJS)
	$(CC) -o $@ $(BUDDY_OBJS) -shared

$(BUDDY_OBJS): %.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES_BUDDY) 

cudd: $(CUDD_DLL_NAME)

$(CUDD_DLL_NAME): $(CUDD_OBJS)
	$(CC) -o $@ $(CUDD_OBJS) -shared

$(CUDD_OBJS): %.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES_CUDD) 

clean:
	$(RM) -f $(BUDDY_OBJS) $(BUDDY_DLL_NAME) $(CUDD_OBJS) $(CUDD_DLL_NAME) 
