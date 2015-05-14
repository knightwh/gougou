# gougou

Gougou Quick Start:

This document shows the step-by-step guide on how to set up and use the gougou search engine: build the index, process queries and evaluate returned results.

Step 1: Download the code
Download the sample.tar.gz attached in this email and untar it
> tar xvf sample.tar.gz
> cd sample

Step 2: Setup and compilation
Change to the gougou directory (where the main files are saved) to set up the environment
> cd gougou
Build all libraries needed to compile the program
> make -f MakefileLibAll.app
Build the program to construct index
> make -f Makefile.app
Build the program to process queries
> make -f MakefileRetrieval.app

Step 3:  Build index from input data
> bin/BuildIndex para/doe.para

Note: the para/doe.para contains the following index and data paths: 
IndexPath: ../index_doe
DataPath: ../doe.trec
If you want to index other files, you can modify the paths in “para/doe.para”. The input file format should be the same as doe.trec. The doe.trec follows the standard TREC format.

Step 4: Process sample queries
> bin/retrieval ../index_doe ../qry ../output 1000
This will run BM25 query evaluation for sample queries (return top 1000 documents for each query).

Note:
The ‘qry’ file contains sample queries as follows: 
<DOC 11>
space
program
</DOC>
<DOC 12>
water
pollution
</DOC>
...

‘output’ is the output file name specified and “1000” is the number of documents you want to return. 


Step 5: Evaluate the results
In this step, we will use the users’ evaluation for the documents to determine the quality of the returned results. Users’ evaluation for documents are saved in the ../qrel file.

Below output the evaluation via MAP and Precision@10 (MP10) method. 
> perl ../whireval.pl ../qrel ../output outFormat=MP10


This will return the following results. It shows the MAP scores and Precision@10 for each query # (topic) in the ‘qry’ file.

topic	M	P10
11	0.0473	0.0000
12	0.0644	0.1000
13	0.8481	1.0000
17	0.2000	0.1000
24	0.0003	0.0000
25	0.0402	0.0000
27	0.0553	0.0000
42	0.0154	0.0000
43	0.0461	0.1000
63	0.1936	0.2000
65	0.3990	0.6000
66	0.4558	0.8000
...



