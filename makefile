include ./make.inc
SRC_DIR = acms
INC_DIR = acms
TEST_DIR = test
ACMSLIB = libacms.$(AR_LIB_SUFFIX)

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG
endif

lib: $(SRC_DIR)/offline_revolve.c $(SRC_DIR)/offline_acms.c $(SRC_DIR)/offline_schedule.h
	$(CC) -c $(CFLAGS) $(SRC_DIR)/offline_revolve.c
	$(CC) -c $(CFLAGS) $(SRC_DIR)/offline_acms.c
	$(AR) $(ARFLAGS) $(ACMSLIB) offline_revolve.o offline_acms.o
	$(RANLIB) $(ACMSLIB)

clean:
	$(RM) $(ACMSLIB) *.o

install:
	$(MKDIR) $(PREFIX)/include $(PREFIX)/lib
	$(CP) $(INC_DIR)/offline_schedule.h $(PREFIX)/include
	$(CP) $(ACMSLIB) $(PREFIX)/lib
	$(RANLIB) $(PREFIX)/lib/$(ACMSLIB)

revolve_binomial.o: $(TEST_DIR)/revolve_binomial.cpp $(INC_DIR)/offline_schedule.h
	$(CXX) $(CXXFLAGS) -c $(TEST_DIR)/revolve_binomial.cpp -I$(INC_DIR)

revolve_test: revolve_binomial.o
	$(CXX) -o revolve_binomial revolve_binomial.o $(ACMSLIB)

dp_test.o: $(TEST_DIR)/dp_test.c $(INC_DIR)/offline_schedule.h
	$(CC) $(CFLAGS) -c $(TEST_DIR)/dp_test.c -I$(INC_DIR)

dp_test: dp_test.o
	$(CC) -o dp_test dp_test.o $(ACMSLIB)

.PHONY:clean
