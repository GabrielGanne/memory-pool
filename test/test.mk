TEST_HEADERS = test/check.h

TEST_SOURCES_MPOOL = test/test_mpool.c
TEST_OBJECTS_MPOOL = $(TEST_SOURCES_MPOOL:.c=.o)
ALL_TEST_OBJECTS = $(TEST_OBJECTS_MPOOL)

.INTERMEDIATE: $(TEST_OBJECTS_MPOOL)
test_mpool: $(TEST_OBJECTS_MPOOL) $(TEST_HEADERS) $(TARGET)
	$(CC) $(CFLAGS) $(LDFLAGS) -L. -lmpool -o $@ $<

TEST_SOURCES_MTHREAD_MPOOL = test/test_mthread_mpool.c
TEST_OBJECTS_MTHREAD_MPOOL = $(TEST_SOURCES_MTHREAD_MPOOL:.c=.o)
ALL_TEST_OBJECTS += $(TEST_OBJECTS_MTHREAD_MPOOL)

.INTERMEDIATE: $(TEST_OBJECTS_MTHREAD_MPOOL)
test_mthread_mpool: $(TEST_OBJECTS_MTHREAD_MPOOL) $(TEST_HEADERS) $(TARGET)
	$(CC) $(CFLAGS) $(LDFLAGS) -L. -lmpool -lpthread -o $@ $<

TEST_SOURCES_MPOOL_OVERLOAD = test/test_mpool_overload.c
TEST_OBJECTS_MPOOL_OVERLOAD = $(TEST_SOURCES_MPOOL_OVERLOAD:.c=.o)
ALL_TEST_OBJECTS += $(TEST_OBJECTS_MPOOL_OVERLOAD)

.INTERMEDIATE: $(TEST_OBJECTS_MPOOL_OVERLOAD)
test_mpool_overload.so: $(OBJECTS) $(TEST_OBJECTS_MPOOL_OVERLOAD)
	$(CC) -shared -fPIC $(CFLAGS) -ldl $(LDFLAGS) -o $@ $^

TEST_SOURCES_SYSTEM_ALLOCS = test/test_system_allocs.c
TEST_OBJECTS_SYSTEM_ALLOCS = $(TEST_SOURCES_SYSTEM_ALLOCS:.c=.o)
ALL_TEST_OBJECTS += $(TEST_OBJECTS_SYSTEM_ALLOCS)

.INTERMEDIATE: $(TEST_OBJECTS_SYSTEM_ALLOCS)
test_system_allocs: $(TEST_OBJECTS_SYSTEM_ALLOCS) $(TEST_HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

ALL_TESTS = \
	test_mpool \
	test_mthread_mpool

TEST_MPOOL_OVERLOAD = test_mpool_overload.so
TEST_SYSTEM_ALLOCS = test_system_allocs

.PHONY: test_clean
test_clean:
	-@rm -vf $(ALL_TESTS)
	-@rm -vf $(ALL_TEST_OBJECTS)
	-@rm -vf $(TEST_MPOOL_OVERLOAD)
	-@rm -vf $(TEST_SYSTEM_ALLOCS)