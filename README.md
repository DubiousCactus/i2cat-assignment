# Point Cloud Transformation and Rendering

This is my i2cat assignment submission.

## 1 - Point cloud manipulation and rendering
For part 1, I chose to use OpenGL and apply per-point transformations in a shader
program, since this is the most performant solution I can think of. I already
implemented a full-featured 3D renderer with OpenGL for a [personal
project](https://github.com/DubiousCactus/gunter), therefore I'll adopt a similar
approach here. Since that project is implemented in Zig and is thus outside the scope of
this assignment, I re-implemented the basic functionality needed for this in C++.


### Data structure

For the point cloud data structure, I went for a simple approach of storing each point
as the suggest struct and aggregating them into a c++ vector. This vector of Point
structs is wrapped within a PointCloud class, which implemented the data loading onto
the GPU via Vertex Buffer Objects (VBO) and friends.


### Transformations

The point transformations are stored as 4x4 matrices ($[R|t], R \in \mathbb{R}^{3\times 3}, t \in \mathbb{R}^{3}$) built
with GLM. These matrices are uploaded to the GPU to be used within the Shader program.
This Shader program efficiently applies the transformation on each point, making use of
extreme parallelism of the GPU pipeline.


### Rendering

Each point is rendered as a set of $s\times s$ pixels, where $s$ is the square root size
of the point. In the Shader program, we ...


## 2 - Particle system


## 3 - NeRF point cloud extraction
