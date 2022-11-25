
TARGET_MODULE := My_proc

obj-m := $(TARGET_MODULE).o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
NUM_THREADS := 4

all: 
	gcc -pthread -o MT_matrix MT_matrix.c
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod $(TARGET_MODULE).ko
unload:
	sudo rmmod $(TARGET_MODULE) || true >/dev/null


1:
	$(MAKE) unload
	$(MAKE) load
	./MT_matrix $(NUM_THREADS) $(PWD)/Test_case_1/m1.txt $(PWD)/Test_case_1/m2.txt
	$(MAKE) unload
2:
	$(MAKE) unload
	$(MAKE) load
	./MT_matrix $(NUM_THREADS) $(PWD)/Test_case_2/m1.txt $(PWD)/Test_case_2/m2.txt
	$(MAKE) unload
3:
	$(MAKE) unload
	$(MAKE) load
	./MT_matrix $(NUM_THREADS) $(PWD)/Test_case_3/m1.txt $(PWD)/Test_case_3/m2.txt
	$(MAKE) unload
4:
	$(MAKE) unload
	$(MAKE) load
	./MT_matrix $(NUM_THREADS) $(PWD)/Test_case_4/m1.txt $(PWD)/Test_case_4/m2.txt
	$(MAKE) unload	