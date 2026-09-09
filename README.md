# LAYR

## Embedding Trustworthiness into Digital Forensic Analysis: A Generalized LAYRed Model

---

## Overview

LAYR is a formal model for forensic data analysis and interpretation. It provides a structured way to describe how forensic data are processed and interpreted across different abstraction levels while keeping the origin of the analyzed data traceable throughout the analysis. This repository contains the open-source implementation of the LAYR model in C++ and Python.

The repository contains the implementation of the LAYR model's basic components, including the base classes for rules and operators. It further provides several concrete rule implementations for forensic data analysis, as well as test input data for experimenting with the LAYR framework and evaluating its functionality.

In addition, the repository contains the example analysis chains presented in Fig. 4 and 5 in the paper _"Embedding Trustworthiness into Digital Forensic Analysis: A Generalized LAYRed Model"_ as directly executable LAYR analyses. These examples demonstrate how the implemented components can be combined to perform complex forensic analysis tasks.

---

## Setup

### Test Images

Before building, extract `TestImages.zip` into the `Tool` folder so the example programs and tests can find the required disk images:

```bash
cd Tool
unzip TestImages.zip
```

This should result in a `Tool/TestImages/` directory containing the test image subfolders (e.g. `DOS`, `EXT3`, `EXT4`, `FAT32`, `NTFS`).

---

## Build

Please use CMake for building LAYR: cmake.org/runningcmake

---

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

---

## Visualizing the Address Tree (Graphviz)

The `example_figure4` example exports its internal `AddressNode` tree to a [Graphviz](https://graphviz.org/) `.dot` file, letting you visually inspect how rules and operators combined to produce the analysis results.

To visualize the `.dot` file in VS Code:

1. Install the extension **Graphviz Interactive Preview** (or **Graphviz Preview**) from the Extensions view.
2. Open your generated `.dot` file in VS Code.
3. Click the preview button in the top-right editor bar.

---

## Documentation

The documentation for this project can be created by going to the 'docs' folder and running 'doxygen'. Afterwards navigate to the 'docs/html' folder and open 'index.html' in a web browser.

---

## Notes

- All datasets are provided for research and reproducibility purposes.
- Please refer to the paper for full methodological details.

---

## Citation

If you use this dataset or materials in your research, please cite the associated paper:

> _Embedding Trustworthiness into Digital Forensic Analysis: A Generalized LAYRed Model_

---

## Contact

For questions or collaboration inquiries, please contact the authors via the corresponding paper affiliation.
