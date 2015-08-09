# psgraph
Automatically exported from code.google.com/p/psgraph
# pGraph - Parallel Similarity Graph Construction using MPI

Google code: https://code.google.com/p/psgraph/

Abstract — Detecting sequence homology between protein sequences is a fundamental problem in computational molecular biology, with a pervasive application in nearly all analyses that aim to structurally and functionally characterize protein molecules. While detecting the homology between two protein sequences is relatively inexpensive, detecting pairwise homology for a large number of protein sequences can become computationally prohibitive for modern inputs, often requiring millions of CPU hours. Yet, there is currently no robust support to parallelize this kernel. In this paper, we identify the key characteristics that make this problem particularly hard to parallelize, and then propose a new parallel algorithm that is suited for detecting homology on large data sets using distributed memory parallel computers. Our method, called pGraph, is a novel hybrid between the hierarchical multiple-master/worker model and producer-consumer model, and is designed to break the irregularities imposed by alignment computation and work generation. Experimental results show that pGraph achieves linear scaling on a 2,048 processor distributed memory cluster for a wide range of inputs ranging from as small as 20,000 sequences to 2,560,000 sequences. In addition to demonstrating strong scaling, we present an extensive report on the performance of the various system components and related parametric studies.

--------------------------------------------------------------------------------------------
Reference:

- pGraph Nonblocking version
 

C. Wu, A. Kalyanaraman, W. Cannon. pGraph: Efficient parallel construction of large-scale protein sequence homology graphs. IEEE Transactions on Parallel and Distributed Systems (TPDS), doi: http://doi.ieeecomputersociety.org/10.1109/TPDS.2012.19.

- pGraph I/O version

C. Wu, A. Kalyanaraman, W. Cannon. A scalable parallel algorithm for large-scale protein sequence homology detection. Proc. 39th International Conference on Parallel Processing (ICPP'2010), San Diego, CA. September 13-16. pp. 333-342.
