# Makefile (starter) for Mission Incomputable

MAKE = make

# These targets do not produce real "targets"
# .PHONY: all clean start-proxy stop-proxy

############## default: make all libs and programs ##########

all: 
	@echo "Making everything..."
<<<<<<< HEAD
	$(MAKE) -C field_agent
	$(MAKE) -C guide_agent
	$(MAKE) -C game_server
=======
	cd guide_agent; $(MAKE)
	cd lib; $(MAKE)
	cd common; $(MAKE)
	cd game_server; $(MAKE)
>>>>>>> e59107d1c22458a65b3282f19e6c6dfb4ceb0541

########################### proxy ###########################

# Start the proxy running - add flags as desired. Options: -all, -fa2gs, -gs2fa
# start-proxy: proxy/proxy
# 	@cd proxy; node proxy

# # Kill any running instances of the proxy.
# stop-proxy:
# 	@pkill -f 'node proxy'

# # Use npm to install all proxy dependencies (only need to do this initially).
# install-proxy-deps:
# 	@cd proxy; npm install

########################### proxy ###########################

clean:
	@echo "Cleaning everything..."
<<<<<<< HEAD
	$(MAKE) -C field_agent clean
	$(MAKE) -C guide_agent clean
	$(MAKE) -C game_server clean
	rm -f *~ *.o
=======
	rm -f *~
	rm -f *.o
	cd lib; $(MAKE) clean
	cd common; $(MAKE) clean
	cd field_agent; $(MAKE) clean
	cd guide_agent; $(MAKE) clean
	cd game_server; $(MAKE) clean
>>>>>>> e59107d1c22458a65b3282f19e6c6dfb4ceb0541
