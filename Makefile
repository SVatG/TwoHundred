3DS_IP := 10.1.1.28
#3DS_IP := 192.168.6.217

CITRA := /home/halcyon/Desktop/3DS/citra/build/src/citra_qt/citra-qt

all: test

clean:
	$(MAKE) -C build clean
	
binary:
	$(MAKE) -C build all
	cp build/*.3dsx .
	
upload: binary
	3dslink -a $(3DS_IP) *.3dsx
	
test: binary
	$(CITRA) *.3dsx