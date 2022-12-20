CFLAGS += -I../../include

%.a: $(OBJ)
	$(AR) rc $@ $<
	ranlib $@

all : $(PLUGIN)

clean:
	rm -f $(PLUGIN) $(OBJ)