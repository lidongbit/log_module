src = $(wildcard *.c)        
obj = $(patsubst %.c, %.o, $(src))  

target=consumer.out
all:$(target)

$(target):$(obj)
	gcc $(^) -o $(@)   

%.o:%.c
	gcc -c $(^) -o $(@)
	
.PHONY:clean all
clean:
	-rm -rf $(target) $(obj)

 
