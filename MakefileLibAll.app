LIBLIST=DAM DAMreader DiskBuffer DiskBufferReader DiskItem DiskItemReader DiskMultiItem DiskMultiItemReader DiskContentReader DocNameLookup DocumentTFGenerator IndexBuilderBase IndexBuilder IndexReaderBase InvertIndexBuilder InvertIndexReader Parser PForCompressor PForDecompressor porter PrefixTree PrefixTreeReader TermMeta TermNameLookup NameLookup NameLookupReader VectorInfo VectorInfoReader
INC=./include
SRCPATH=./src
LIBPATH=./lib

CXX=g++
CXXFLAGS = -O3 -I$(INC) -c
CPPLDFLAGS = -lz -lpthread -lm

all:
	for ITEM in  $(LIBLIST) ; do \
		echo "$(CXX) $(CXXFLAGS) $(SRCPATH)/$${ITEM}.cpp -o $(LIBPATH)/$${ITEM}.o" ; \
		$(CXX) $(CXXFLAGS) $(SRCPATH)/$${ITEM}.cpp -o $(LIBPATH)/$${ITEM}.o ; \
	done ;

clean:
	for ITEM in $(LIBLIST) ; do \
		rm -f $(LIBPATH)/$${ITEM}.o ; \
	done ;

