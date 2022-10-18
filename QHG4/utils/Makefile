
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

include ../craydefs.mk
include ../cflags.mk
include ../codedirs.mk

ifdef MUTE
  MUTEFLAG=-D MUTE
else
  MUTEFLAG=
endif


ifndef WARNINGS
  WARNINGS=-Wall 
endif

ifdef LBOLD
  DEFS=-D LBOLD
endif

COPT=-c $< -o $@ $(CADD) $(CFLAGS) $(DYN) -fPIC $(C_STD) $(CRAY_DEF) $(COMP) $(DEFS)


$(warning DEFS in utils is $(DEFS))

TESTS=BinoDistTest L2LTest LABTest LBTest SysInfoTest  WELLTest stdsprintf_test

ARFL=rv


lib: libUtils.a

all: libUtils.a

utils: libUtils.a


libUtils.a: utils.o strutils.o stdstrutils.o geomutils.o calcutils.o PolyLine.o gzutils.o \
      LineReader.o \
      Pattern.o FileList.o  \
      Quat.o \
      Vec3D.o \
      smallprimes.o \
      MessLogger.o ParamReader.o dbgprint.o \
      BufWriter.o BufReader.o \
      Observable.o \
      LayerBuf.o LayerArrBuf.o L2List.o LBController.o \
      SeqInterval.o \
      WELL512.o WELLUtils.o bino_tools.o BinomialDist.o Permutator.o \
      SimpleMapper.o crypto.o crc.o \
      SystemInfo.o ExecuteCommand.o \
      ArrayShare.o \
      RankTable.o
	ar $(ARFL) $@ $?


test: $(TESTS)


clean:
	rm -f *.o libUtils.a *.ti $(TESTS)



utils.o: utils.cpp  utils.h 
	$(CC) $(COPT) $(DEFS)

strutils.o: strutils.cpp  strutils.h 
	$(CC) $(COPT) $(DEFS) -fPIC

stdstrutils.o: stdstrutils.cpp  stdstrutils.h
	$(CC) $(COPT) $(DEFS) -fPIC	


geomutils.o: geomutils.cpp  geomutils.h utils.h
	$(CC) $(COPT) $(DEFS)

calcutils.o: calcutils.cpp  calcutils.h PolyLine.h
	$(CC) $(COPT) $(DEFS)

gzutils.o: gzutils.cpp  gzutils.h
	$(CC) $(COPT) $(DEFS)

LineReader.o: LineReader.cpp  LineReader.h  utils.h
	$(CC) $(COPT) $(DEFS)

Vec3D.o: Vec3D.cpp  Vec3D.h
	$(CC) $(COPT) $(DEFS)

Quat.o: Quat.cpp  Quat.h Vec3D.h
	$(CC) $(COPT) $(DEFS)

MessLogger.o: MessLogger.cpp MessLogger.h MessLoggerT.h stdstrutils.h stdstrutilsT.h
	$(CC) $(COPT) $(DEFS) -fPIC

ParamReader.o: ParamReader.cpp ParamReader.h stdstrutils.h stdstrutilsT.h
	$(CC) $(COPT) $(DEFS) -fPIC

Pattern.o: Pattern.cpp Pattern.h
	$(CC) $(COPT) $(DEFS)

FileList.o: FileList.cpp FileList.h Pattern.h
	$(CC) $(COPT) $(DEFS)

PolyLine.o: PolyLine.cpp  PolyLine.h 
	$(CC) $(COPT) $(DEFS)

SimpleMapper.o: SimpleMapper.cpp  SimpleMapper.h ValueMapper.h 
	$(CC) $(COPT) $(DEFS)


dbgprint.o: dbgprint.cpp  dbgprint.h
	$(CC) $(COPT) $(MUTEFLAG) $(DEFS) -g 

WELL512.o: WELL512.cpp WELL512.h 
	$(CC) $(COPT) $(DEFS) 

WELLUtils.o: WELLUtils.cpp WELLUtils.h WELL512.h 
	$(CC) $(COPT) $(DEFS) 

WELLTest.o: WELLTest.cpp WELL512.h 
	$(CC) $(COPT) $(DEFS) 

WELLTest: WELLTest.o WELL512.o libUtils.a
	g++ WELLTest.o WELL512.o  libUtils.a \
          -o WELLTest $(COMP)

SystemInfo.o: SystemInfo.cpp SystemInfo.h
	$(CC) $(COPT) $(DEFS)

SysInfoTest.o: SysInfoTest.cpp SystemInfo.h
	$(CC) $(COPT) $(DEFS)

SysInfoTest:SystemInfo.o SysInfoTest.o
	g++ SystemInfo.o SysInfoTest.o strutils.o \
          -o SysInfoTest

ExecuteCommand.o: ExecuteCommand.cpp ExecuteCommand.h
	$(CC) $(COPT) $(DEFS)

BinomialDist.o: BinomialDist.cpp BinomialDist.h bino_tools.h
	$(CC) $(COPT) $(DEFS)

BinoDistTest.o: BinoDistTest.cpp BinomialDist.h
	$(CC) $(COPT) $(DEFS)

bino_tools.o: bino_tools.cpp bino_tools.h
	$(CC) $(COPT) $(DEFS)

BinoDistTest: BinoDistTest.o BinomialDist.o bino_tools.o
	g++ BinoDistTest.o BinomialDist.o bino_tools.o -o BinoDistTest

Permutator.o: Permutator.cpp Permutator.h
	$(CC) $(COPT) $(DEFS)

SeqInterval.o: SeqInterval.cpp SeqInterval.h utils.h types.h
	$(CC) $(COPT) $(DEFS)

smallprimes.o: smallprimes.cpp smallprimes.h 
	$(CC) $(COPT) $(DEFS)

crypto.o: crypto.cpp crypto.h 
	$(CC) $(COPT) $(DEFS)

crc.o: crc.cpp crc.h 
	$(CC) $(COPT) $(DEFS)

BufWriter.o: BufWriter.cpp BufWriter.h 	
	$(CC) $(COPT) $(DEFS)


BufReader.o: BufReader.cpp BufReader.h 	
	$(CC) $(COPT) $(DEFS)


Observable.o: Observable.cpp Observable.h Observer.h 	
	$(CC) $(COPT) $(DEFS)


LayerBuf.o: LayerBuf.cpp LayerBuf.h LBBase.h 	
	$(CC) $(COPT) $(DEFS)

LayerArrBuf.o: LayerArrBuf.cpp LayerArrBuf.h LayerBuf.h LayerBuf.cpp LBBase.h 	
	$(CC) $(COPT) $(DEFS)

LBController.o: LBController.cpp LBController.h LayerBuf.h LBBase.h
	$(CC) $(COPT) $(DEFS)

LBTest.o: LBTest.cpp L2List.h LayerBuf.h LayerBuf.cpp LBController.h
	$(CC) $(COPT) $(DEFS)

LBTest: LBTest.o libUtils.a 
	g++ LBTest.o libUtils.a -o LBTest


LABTest: LABTest.o libUtils.a 
	g++ LABTest.o libUtils.a -o LABTest

LABTest.o: LABTest.cpp L2List.h LayerArrBuf.h LayerBuf.h LayerBuf.cpp LBController.h
	$(CC) $(COPT) $(DEFS)



L2List.o: L2List.cpp L2List.h 
	$(CC) $(COPT) $(DEFS)

L2LTest.o: L2LTest.cpp L2List.h
	$(CC) $(COPT) $(DEFS)

L2LTest: L2LTest.o L2List.o strutils.o
	g++ L2LTest.o L2List.o strutils.o -o L2LTest

ArrayShare.o: ArrayShare.cpp ArrayShare.h
	$(CC) $(COPT) $(DEFS) 

ASTest.o: ASTest.cpp ArrayShare.h
	$(CC) $(COPT) $(DEFS)

ASTest: ASTest.o ArrayShare.o
	$(CC) -o ASTest \
	ASTest.o ArrayShare.o Pattern.o


RankTable.o: RankTable.cpp RankTable.h
	$(CC) $(COPT) $(DEFS) 

RTTest.o: RTTest.cpp RankTable.h
	$(CC) $(COPT) $(DEFS) 

RTTest: RTTest.o RankTable.o
	$(CC) -o RTTest \
	RTTest.o RankTable.o \
	-L$(UTILS) -lUtils 

stdsprintf_test.o: stdsprintf_test.cpp 
	$(CC) $(COPT) $(DEFS) 

stdsprintf_test: stdsprintf_test.o
	$(CC) -o stdsprintf_test \
	stdsprintf_test.o \
        -L$(UTILS) -lUtils
