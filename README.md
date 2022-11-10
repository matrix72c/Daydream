# Intro
Experimental implementation for paper [Daydream: Accurately Estimating the Efficacy of Optimizations for DNN Training](https://www.usenix.org/conference/atc20/presentation/zhu-hongyu).

# Requirements
- CUDA 11.7
- linux-tools-aws
- python3-dev
- libunwind-dev

Test on Ubuntu 20.04, Python 3.9, PyTorch 1.13.0, CUDA 11.7.1

# Usage
Import daydream, start trace at the beginning of your code and end trace at the end of your code. For an example, see `train.py`.

# Roadmap
- [x] Trace six activities, including Memory Copy, Memory Set, Kernel Execution, CUDA Driver and CUDA Runtime.
- [x] Packaged as PyThon extension.
- [ ] Add trace visualization and analysis tools.
- [ ] Modify DNN framework for kernel to layer mapping.
- [ ] Construct dependency graph and expose graph trasformation APIs for users to manipulate the graph.

# Reference
[NVIDIA/cuda-samples](https://github.com/NVIDIA/cuda-samples)

[NihalHarish/cupti-tracer](https://github.com/NihalHarish/cupti-tracer)

[CUDA Driver API vs. CUDA runtime](https://stackoverflow.com/questions/242894/cuda-driver-api-vs-cuda-runtime)