# ECX Testing Harness (ETH) 

<img src="https://github.com/ascr-ecx/ecx-harness/blob/master/img/icon.png" width="100"/>

**Contact:** Greg Abram gda@tacc.utexas.edu (primary developer)

This is the ECX Test Harness project - a lightweight test harness for evaluating analysis and visualization workflows under different parallel work configurations and rendering options.

Testing individual codes for visualization and analysis workflows – though accurate – presents difficulties in compilation, flexibility and adaptiveness to new experiments.

We developed this lightweight test harness (ETH) that can load domain-specific data (for example, from a simulation), and perform a range of analysis and visualization operations in different configurations and with different visualization pipelines to promote comparison of a wide range of experiments.

* Paired simulation and visualization processes can each run different sets of analysis and visualization operations on data read from disk.

* ETH runs many copies of the sim/vis process pairings, in a number of different configuraitons: single process, multi-core, and multi-node.

* Workflows are based on VTK, so analysis and vis operations can easily be modified to accommodate specific experiments.

* Multiple rendering pipelines (ray traced, and geometry-based) are currently supported.

## Configurations

ETH can be run in several configurations, to explore different distributions of execution.

* **Coupled** The simulation process and the vis/analysis process run in the same process.
* **Core-to-core** The simulation process and the vis/analysis process run on the same node, but on different cores.
* **Node-to-node** The simulation process and the vis/analysis process run on different nodes.

## Acknowledgement 

This material is based upon work supported by **Dr. Lucy Nowell** of the **U.S. Department of Energy Office of Science, Advanced Scientific 
Computing Research** under Award Numbers DE-AS52-06NA25396 and DE-SC-0012516. It is an artifact created for the ECX project (http://www.ecxproject.org)

## License

This code is licensed under a BSD 3-Clause License. Copyright (c) 2016, University of Texas at Austin, Los Alamos National Laboratory. All rights reserved. 

The icon for this project was downloaded from The Noun Project (www.nounproject.com), and was created by Tamiko Young.
