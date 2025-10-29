CC=gcc
SRC=\
  mgr.c \
  Mandelbrot.c \
  MandelbrotMasterSlave.c \
  MatrixDeterminant.c \
  MatrixDeterminantMasterSlave.c \
  MergeSort.c \
  MergeSortMasterSlave.c

OUT_RELEASE=mgr_release
OUT_DEBUG=mgr_debug
OUT_ASAN=mgr_asan

CFLAGS_RELEASE=-O2 -Wall -fopenmp
CFLAGS_DEBUG=-g -D_DEBUG -Wall -fopenmp
LDFLAGS=-lm

ASAN_FLAGS=-O1 -g -D_DEBUG -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all
LDFLAGS_ASAN=-fsanitize=address,undefined -lm -fopenmp

all: release

release:
	$(CC) $(CFLAGS_RELEASE) -o $(OUT_RELEASE) $(SRC) $(LDFLAGS)

debug:
	$(CC) $(CFLAGS_DEBUG) -o $(OUT_DEBUG) $(SRC) $(LDFLAGS)

asan:
	$(CC) $(ASAN_FLAGS) -o $(OUT_ASAN) $(SRC) $(LDFLAGS_ASAN)

clean:
	rm -f $(OUT)