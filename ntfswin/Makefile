
SOURCE = $(wildcard *.c)  
OBJS = $(patsubst %.c,%.o,$(SOURCE)) 

CXX = gcc
INCLUDE= -I.
CFLAGS += $(INCLUDE) -g -D__BYTE_ORDER="__LITTLE_ENDIAN" -D__LITTLE_ENDIAN \
          -DHAVE_CONFIG_H
#CLFAGS += -Wall   
#LDFLAGS += -lm  

all:ntfscp

ntfscp:$(OBJS)
	$(CXX) -Wall $(CFLAGS) -o $@ $^

include $(SOURCE:.c=.d)   
      
%.o:%.c  
	$(CXX) -Wall $(CFLAGS) -c $< -o $@  
  
%.d: %.c  
	@set -e; rm -f $@; \
	$(CXX) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$  

.PHONY:clean
clean:
	-rm -f *.o *.d ntfscp