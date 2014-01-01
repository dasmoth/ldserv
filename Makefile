CXX = g++
CXXFLAGS = -O3 -D_FILE_OFFSET_BITS=64 -std=c++0x

TABIX = tabixpp/tabix.o


all: ldserv vcf2gt

$(TABIX):
	cd tabixpp && $(MAKE)

ldserv: ldserv.cc $(TABIX)
	$(CXX) $(CXXFLAGS) -o $@ ldserv.cc $(TABIX) -Ltabixpp -ltabix -lz -lm

vcf2gt: vcf2gt.cc
	$(CXX) $(CXXFLAGS) -o $@ vcf2gt.cc


clean:
	rm -f ldserv vcf2gt
	cd tabixpp && $(MAKE) clean

.PHONY: clean all