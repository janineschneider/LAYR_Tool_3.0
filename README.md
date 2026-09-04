# LAYR
## Embedding Trustworthiness into Digital Forensic Analysis: A Generalized LAYRed Model 

---

## Overview
LAYR is a formal model for forensic data analysis and interpretation. It provides a structured way to describe how forensic data are processed and interpreted across different abstraction levels while keeping the origin of the analyzed data traceable throughout the analysis. This repository contains the open-source implementation of the LAYR model in C++ and Python.

The repository contains the implementation of the LAYR model's basic components, including the base classes for rules and operators. It further provides several concrete rule implementations for forensic data analysis, as well as test input data for experimenting with the LAYR framework and evaluating its functionality.

In addition, the repository contains the example analysis chains presented in Fig. 4 and 5 in the paper *"Embedding Trustworthiness into Digital Forensic Analysis: A Generalized LAYRed Model"* as directly executable LAYR analyses. These examples demonstrate how the implemented components can be combined to perform complex forensic analysis tasks.

## Build

Please use CMake for building LAYR: cmake.org/runningcmake

## Supported Rules:

- DOS
- FAT32
- NTFS
- EXT3
- EXT4
- Carve (Scalpel based)
- Volatility
- VirMA
- DFXML

## Documentation

The documentation for this project can be created by going to the 'docs' folder and running 'doxygen'. Afterwards navigate to the 'docs/html' folder and open 'index.html' in a web browser.

---

## Notes

- All datasets are provided for research and reproducibility purposes.
- Please refer to the paper for full methodological details.

---

## Citation

If you use this dataset or materials in your research, please cite the associated paper:

> *Embedding Trustworthiness into Digital Forensic Analysis: A Generalized LAYRed Model*

---

## Contact

For questions or collaboration inquiries, please contact the authors via the corresponding paper affiliation.