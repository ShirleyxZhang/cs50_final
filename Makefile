# Makefile (starter) for Mission Incomputable

MAKE = make

# These targets do not produce real "targets"
.PHONY: all clean start-proxy stop-proxy

############## default: make all libs and programs ##########

all: 
	@echo "Making everything..."
	
	cd lib; $(MAKE)
	cd common; $(MAKE)
	cd game_server; $(MAKE)
	cd guide_agent; $(MAKE)

########################### proxy ###########################

Start the proxy running - add flags as desired. Options: -all, -fa2gs, -gs2fa
start-proxy: proxy/proxy
	@cd proxy; node proxy

# Kill any running instances of the proxy.
stop-proxy:
	@pkill -f 'node proxy'

# Use npm to install all proxy dependencies (only need to do this initially).
install-proxy-deps:
	@cd proxy; npm install

########################### proxy ###########################

clean:
	@echo "Cleaning everything..."
	rm -f *~
	rm -f *.o
	cd lib; $(MAKE) clean
	cd common; $(MAKE) clean
	cd guide_agent; $(MAKE) clean
	cd game_server; $(MAKE) clean