ifeq ($(KERNELRELEASE),)  

KERNELDIR ?= /lib/modules/$(shell uname -r)/build 
PWD := $(shell pwd)  

.PHONY: build clean  

build:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

attack:
	gcc -Werror -Wall -pedantic -ggdb3 -std=gnu99 -o sneaky_process sneaky_process.c



clean:
	rm -rf *.o *~ core .depend .*.cmd *.order *.symvers *.ko *.mod.c
aclean:
	rm -f sneaky_process
else  

$(info Building with KERNELRELEASE = ${KERNELRELEASE}) 
obj-m :=    sneaky_mod.o  

endif
