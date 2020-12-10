include ./make.inc
SRC_DIR = src
INC_DIR = src
TEST_DIR = test
REVOLVELIB = libmrevolve.$(AR_LIB_SUFFIX)

lib: $(SRC_DIR)/offline_revolve.c $(SRC_DIR)/offline_acms.c $(SRC_DIR)/offline_schedule.h
	$(CC) -c $(CFLAGS) $(SRC_DIR)/offline_revolve.c
	$(CC) -c $(CFLAGS) $(SRC_DIR)/offline_acms.c
	$(AR) $(ARFLAGS) $(REVOLVELIB) offline_revolve.o offline_acms.o
	$(RANLIB) $(REVOLVELIB)

clean:
	$(RM) $(REVOLVELIB) *.o
	
install:
	$(MKDIR) $(PREFIX)/include $(PREFIX)/lib
	$(CP) $(INC_DIR)/offline_schedule.h $(PREFIX)/include
	$(CP) $(REVOLVELIB) $(PREFIX)/lib
	$(RANLIB) $(PREFIX)/lib/$(REVOLVELIB)

revolve_binomial.o: $(TEST_DIR)/revolve_binomial.cpp $(INC_DIR)/offline_schedule.h
	$(CXX) $(CXXFLAGS) -c $(TEST_DIR)/revolve_binomial.cpp -I$(INC_DIR)

revolve_test: revolve_binomial.o
	$(CXX) -o revolve_binomial revolve_binomial.o $(REVOLVELIB)

dp_test.o: $(TEST_DIR)/dp_test.c $(INC_DIR)/offline_schedule.h
	$(CC) $(CFLAGS) -c $(TEST_DIR)/dp_test.c -I$(INC_DIR)

dp_test: dp_test.o
	$(CC) -o dp_test dp_test.o $(REVOLVELIB)

.PHONY:clean
