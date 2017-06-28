#Targets:
#  Default target: build project
#  clean:          remove all generated files.
#  submit:         build compress archive all project source files.

PROJECT = 	prj4

TARGET =	word-countd word-count

#Files needed for client and server.  List as per your organization.
#Should have a .h file which contains information common to both 
#client and server.
PROG_FILES = \
  arrayi.h \
  compare-fn.h \
  common.c \
  common.h \
  dynamic-array.c \
  dynamic-array.h \
  errors.c \
  errors.h \
  file-words.c \
  file-words.h \
  hash-map.c \
  hash-map.h \
  heapsort.c \
  heapsort.h \
  interfaces.h \
  linked-list-map.c \
  linked-list-map.h \
  main.c \
  mapi.h \
  map-unit-test.c \
  map-unit-test.h \
  memalloc.c \
  memalloc.h \
  realloc-buf.c \
  realloc-buf.h \
  string-hashcode.c \
  string-hashcode.h \
  string-reader.c \
  string-reader.h \
  strspace.c \
  strspace.h \
  unit-test.c \
  unit-test.h \
  word-count.c \
  word-count.h \
  word-countd.c \
  word-counts.h \
  worker.c \
  worker.h

#All files to be submitted
SRC_FILES = \
  $(PROG_FILES) \
  Makefile \
  README


#List name of all objects needed for building server.
SERVER_OBJS = \
  dynamic-array.o \
  common.o \
  data-buffer.o \
  errors.o \
  file-words.o \
  hash-map.o \
  heapsort.o \
  linked-list-map.o \
  memalloc.o \
  realloc-buf.o \
  string-hashcode.o \
  string-reader.o \
  strspace.o \
  word-countd.o \
  word-counts.o \
  worker.o

CLIENT_OBJS = \
  common.o \
  data-buffer.o \
  errors.o \
  memalloc.o \
  word-count.o 

LIB_FILES = \
  dynamic-array.o \
  errors.o \
  file-words.o \
  hash-map.o \
  heapsort.o \
  linked-list-map.o \
  map-unit-test.o \
  memalloc.o \
  realloc-buf.o \
  string-hashcode.o \
  string-reader.o \
  strspace.o \
  unit-test.o \
  word-counts.o \
  worker.o

LIB = $(PROJECT).a

CFLAGS = -g -Wall -std=c11 -fPIC -D_POSIX_C_SOURCE=200809L
LDFLAGS= -lpthread -lrt -lm

all:			$(TARGET)

word-countd:		$(SERVER_OBJS) trace.h
			$(CC) $^  $(LDFLAGS) -o $@

word-count:		$(CLIENT_OBJS) trace.h
			$(CC) $^  $(LDFLAGS) -o $@

clean:		
			rm -f $(LIB_FILES) $(LIB) $(TARGET) *~ test-* \
			      *.o 

$(LIB):			$(LIB_FILES)
			ar rcs $@ $^

submit:
			tar -cvzf $(PROJECT).tar.gz $(SRC_FILES)

UNIT_TESTS = \
  test-file-words \
  test-linked-list-map \
  test-dynamic-array \
  test-hash-map \
  test-heapsort \
  test-realloc-buf \
  test-string-reader \
  test-strspace \
  test-word-counts \

unit-tests:		$(UNIT_TESTS)
			@FAILS='' ; \
			for t in $^ ; \
			do \
			  ./$$t ; \
			  if [ $$? -eq 0 ] ; \
	                  then \
	                    echo $$t ok ; \
	                  else \
	                    echo $$t failed ; \
			    FAILS=" $$FAIL $$t"; \
		          fi ; \
			done; \
			if [ ! -z $$FAILS ] ; \
			then \
			  echo ' ' ; \
			  echo " ***** $$FAILS failed *****" ; \
			  false ; \
			fi


test-file-words:	file-words.c file-words.h $(LIB)
			$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L \
			      -DTEST_FILE_WORDS $< $(LIB) -o $@

test-linked-list-map:	linked-list-map.c linked-list-map.h $(LIB)
			$(CC) $(CFLAGS) -DTEST_LINKED_LIST_MAP $< $(LIB) -o $@

test-dynamic-array:	dynamic-array.c dynamic-array.h $(LIB)
			$(CC) $(CFLAGS) -DTEST_DYNAMIC_ARRAY $< $(LIB) -o $@

test-hash-map:		hash-map.c hash-map.h $(LIB)
			$(CC) $(CFLAGS) -DTEST_HASH_MAP $< $(LIB) -o $@

test-heapsort:		heapsort.c heapsort.h $(LIB)
			$(CC) $(CFLAGS) -DTEST_HEAPSORT $< $(LIB) -o $@

test-realloc-buf:	realloc-buf.c realloc-buf.h $(LIB)
			$(CC) $(CFLAGS) -DTEST_REALLOC_BUF $< $(LIB) -o $@

test-strspace:		strspace.c strspace.h $(LIB)
			$(CC) $(CFLAGS) -DTEST_STRSPACE $< $(LIB) -o $@

test-string-reader:	string-reader.c string-reader.h $(LIB)
			$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L \
			      -DTEST_STRING_READER $< $(LIB) -o $@
test-word-counts:	word-counts.c word-counts.h $(LIB)
			$(CC) $(CFLAGS) -DTEST_WORD_COUNTS $< $(LIB) -o $@


test-worker:		worker.c worker.h data-buffer.o common.o $(LIB)
			$(CC) $(CFLAGS) -DTEST_WORKER $< \
			  data-buffer.o common.o $(LIB) $(LDFLAGS) -o $@

#This target outputs dependencies of all C files on stdout.  The output can
#then be cut-and-pasted into this Makefile.
DEPEND:
		$(CC) $(CPPFLAGS) -MM *.c

alnum-quote.o: alnum-quote.c word-char.h
dynamic-array.o: dynamic-array.c arrayi.h compare-fn.h interfaces.h \
 memalloc.h dynamic-array.h heapsort.h
errors.o: errors.c errors.h
file-words.o: file-words.c file-words.h word-char.h word-counts.h \
 realloc-buf.h string-reader.h
hash-map.o: hash-map.c arrayi.h compare-fn.h interfaces.h dynamic-array.h \
 hash-map.h mapi.h memalloc.h
heapsort.o: heapsort.c arrayi.h compare-fn.h interfaces.h heapsort.h
linked-list-map.o: linked-list-map.c compare-fn.h linked-list-map.h \
 mapi.h interfaces.h memalloc.h
map-unit-test.o: map-unit-test.c mapi.h interfaces.h map-unit-test.h \
 unit-test.h
memalloc.o: memalloc.c errors.h
non-space.o: non-space.c word-char.h
realloc-buf.o: realloc-buf.c memalloc.h realloc-buf.h
string-hashcode.o: string-hashcode.c
string-reader.o: string-reader.c errors.h realloc-buf.h string-reader.h
strspace.o: strspace.c memalloc.h strspace.h string-reader.h \
 realloc-buf.h
unit-test.o: unit-test.c unit-test.h
word-count.o: word-count.c
word-counts.o: word-counts.c dynamic-array.h arrayi.h compare-fn.h \
 interfaces.h memalloc.h strspace.h word-counts.h hash-map.h mapi.h \
 string-hashcode.h
worker.o: worker.c dynamic-array.h arrayi.h compare-fn.h interfaces.h \
 errors.h file-words.h word-char.h word-counts.h



