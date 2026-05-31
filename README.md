# RGB Image Processor & Segmentation - TAD (AED)

##  Overview
This repository contains a professional implementation of an **RGB Image Abstract Data Type (TAD imageRGB)** developed for the **Algoritmos e Estruturas de Dados** course at the **Universidade de Aveiro**. 

The system manipulates 8-bit RGB color images using a memory-optimized architecture based on a 2D pixel matrix that indexes a dynamic **Look-Up Table (LUT)** containing color tuples.

##  Project Architecture & Codebase
The workspace contains all the source components required for modular C compilation and image processing testing:
* **Core TAD (`imageRGB.c` / `imageRGB.h`)**: Lifecycle management, memory allocation tracking, image cloning, and matrix-based spatial rotations ($90^{\circ}$ and $180^{\circ}$).
* **Auxiliary Data Structures (`PixelCoordsStack` & `PixelCoordsQueue`)**: Custom-built stack and queue architectures implemented explicitly to manage coordinate processing iteratively, avoiding runtime call-stack overflows during heavy flood-fill operations.
* **Testing Suite & Assets**: Custom test scripts (`imageRGBTest.c`, `imageRGBTest2.c`) along with source imagery (`.ppm`, `.pbm`) used to benchmark algorithmic thresholds.

##  Algorithmic Complexity Report (`relatorio_P8F.pdf`)
A core requirement of this engineering project was a mathematical and empirical evaluation of the codebase, which is detailed in the included PDF report:
* **Theoretical Big-O Analysis**: Formal deduction of the computational complexity for the image validation logic (`ImageIsEqual`), defining precise mathematical boundaries for both **Best-Case** and **Worst-Case** scenarios based on pixel comparisons.
* **Empirical Benchmarking**: Statistical data tables mapping execution runtimes against scaling image dimensions and color distributions, validating the formal Big-O constraints through experimental testing.
* **Traversal Strategy Comparison**: A structural analysis contrasting the behavioral mechanics of Recursive Flood Fill versus Iterative Queue (BFS) and Stack (DFS) implementations for full image segmentation.

##  Key Algorithmic Implementations
* **`ImageRegionFillingRecursive`**: Recursive 4-connectivity neighbor traversal algorithm for localized region growing.
* **`ImageRegionFillingWithQUEUE` / `WithSTACK`**: High-performance iterative variants leveraging explicit memory buffers.
* **`ImageSegmentation`**: Full raster-scan image partitioning that systematically isolates boundaries and assigns unique LUT indexes across the entire matrix.
