EXTRA_CFLAGS += $(shell rtai-config --module-cflags)

obj-m += multi-timer_rt.o

multi-timer_rt-objs := multi-timer.o
