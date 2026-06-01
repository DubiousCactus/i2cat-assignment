# Assignment submission

This is my i2cat assignment submission.

## 1 - Point cloud manipulation and rendering
To run part 1, please use an OpenGL-compatible environment with cmake, and from the
project directory, run:
```
cmake --build build --target i2cat_p1
./build/i2cat_p1
```
Note that the point cloud file is expected to be in the project root.

For part 1, I chose to use OpenGL and apply per-point transformations in a shader
program, since this is the most performant solution I can think of. I already
implemented a full-featured 3D renderer with OpenGL for a [personal
project](https://github.com/DubiousCactus/gunter), therefore I'll adopt a similar
approach here. Since that project is implemented in Zig and is thus outside the scope of
this assignment, I re-implemented the basic functionality needed for this in C++.


### Data structure

For the point cloud data structure, I went for a simple approach of storing each point
as the suggested struct using the GLM library, and aggregating them into a c++ vector.
This vector of Point structs is wrapped within a PointCloud class, which implemented the
data loading onto the GPU via a Vertex Buffer Object (VBO) and a Vertex Array Object
(VAO).


### Transformations

The point transformations are stored as 4x4 matrices ($[R|t], R \in \mathbb{R}^{3\times 3}, t \in \mathbb{R}^{3}$) built
with GLM. These matrices are uploaded to the GPU to be used within the Shader program.
This Shader program efficiently applies the transformation on each point, making use of
extreme parallelism of the GPU pipeline.


### Rendering

I use OpenGL's point rendering pipeline to draw individual points instead of rasterizing
polygons.


## 2 - Particle system
To run part 2, please use an OpenGL-compatible environment with cmake, and from the
project directory, run:
```
cmake --build build --target i2cat_p2
./build/i2cat_p2
```

For part 2, I used this simple OpenGL engine and extended my PointCloud implementation
to implement a basic 2D particle system. Each point is assigned a random velocity
vector, and on each iteration of the render loop, we check for collisions and react
accordingly. I chose to implement the particle system fully in C++, as opposed to
something like a Compute Shader, since this is more straightforward and perhaps more
realistic in terms of real-world applications with CPU only.

To make the collision check efficient, I went for a simple approach of subdividing the
(normalized coordinate) space into a grid of NxN cells. I then assign each point to its
grid cell, sort the indices by cell assignment order, and keep track of the cell offsets
in the sorted list. This gives me a contiguous array of point indices that belong to the
same cell. I can then efficiently find the neighbours of each point by its current cell
+ the 8 neighbouring cells, resulting in considerably less pairwise distance checks for
each point. With a grid of 30x30 cells, I can simulate upwards of 20,000 points smoothly
on a macbook air M2.

For collision handling, I chose to simply swap the velocity vectors of both particles,
with some added randomness. In addition, I apply a small random magnitude scalar between
0.1 and 2.0, resulting in slightly more chaotic movement to make things more
interesting.

### Optimizations
While the current approach allows up to 20K points to be simulated on a MacBook Air M2,
it could be optimized further. For instance, using a Compute Shader would make better
use of the hardware acceleration to parallelize the computation of collision checks,
potentially unlocking millions of points. A GPU solution could even remove the necessity
of sorting the indices, by pre-computing cell assignments and operating threads on the
cell level, similar to the 3DGS rasterizer.

The CPU solution could also be optimized with parallelism and SIMD instructions. For
instance, each cell could be checked in parallel or up to the number of available
threads, and a group of points could be computed together in one SIMD instruction.
However, to prevent race conditions without mutexes, the updates to the velocity vectors
would need to be applied in a second pass. This could potentially yield better results
than the current implementation which updates the velocity vectors potentially multiple
times per point, without updating the positions.

## 3 - NeRF point cloud extraction

We are looking into the InstantNGP implementation for near real-time NeRFs.

### Where does the rendering take place?

When looking at the Python API/bindings, we can find the `render()` binding at [this
line](https://github.com/NVlabs/instant-ngp/blob/abe236ee00cf90cfca6e36e65c00435d5b21f50a/src/python_api.cu#L146).
This function accumulates multiple samples per pixel from the actual NeRF rendering code
defined in
[testbed.h](https://github.com/NVlabs/instant-ngp/blob/abe236ee00cf90cfca6e36e65c00435d5b21f50a/include/neural-graphics-primitives/testbed.h#L378)
and implemented in
[testbed.cu](https://github.com/NVlabs/instant-ngp/blob/abe236ee00cf90cfca6e36e65c00435d5b21f50a/src/testbed.cu#L4870).
The core of the NeRF rendering is implemented in the function
[render_nerf()](https://github.com/NVlabs/instant-ngp/blob/abe236ee00cf90cfca6e36e65c00435d5b21f50a/include/neural-graphics-primitives/fused_kernels/render_nerf.cuh#L22),
which evaluates the NeRF model for every sample point on the camera ray. Each pixel is
evaluated in parallel using this kernel.


### What algorithms do they use to infer the missing surfaces?
By "to infer the missing surfaces", I suppose that what is meant is "how does
instant-ngp fit a NeRF to sparse observations?".
The main contribution of instant-ngp is to enhance the classical NeRF training algorithm
with a multi-resolution hash grid.

The NeRF model is trained using deep learning and stochastic gradient descent, such that
a neural network learns the radiance field as an implicit function. This function
encodes the density and view-dependent colour (in linear space) as a function of space,
i.e.:
$$
f(x, d) = \left[\sigma\\ c \right]
$$
where $x \in \mathbb{R}^{3}$ is a point in space and $d\int \mathbb{R}^3$ is the view
direction.
The query points and view directions are obtained via ray casting from the camera, and
marching along the ray (with importance sampling) to sample a set of points. The pixel
colour for that ray is then computed by integrating the density and colour predicted
along the ray by the neural network, i.e. the NeRF model. Using Stochastic Gradient
Descent (SGD), the model is trained to minimize the pixel reconstruction error for the
training viewpoints.

Since NeRF is a notoriously slow technique (both during training and inference),
Instant-NGP introduces a multiresolution hash grid to learn sparse features in space, on
multiple resolution levels. Given a point in space, the features of the point's voxel
are interpolated for each resolution level of the grid, and all concatenated to form a
feature vector. This feature vector is used as the input to the NeRF model, instead of
the raw 3D points. Using such data structure improves the training time significantly,
while having a regularizing effect that helps interpolate between the training views.

### How is the view synthesis computed?
As explained above, view synthesis is achieved by integrating the density and
view-dependent colour values along a camera ray, obtained via ray-marching and
evaluating the NeRF at each point on the ray. Typically, multiple samples per pixel are
used to reduce aliasing and blur.


### Mesh extraction from NeRF
Unfortunately, I do not have the time to fit a NeRF and extract the point cloud,
although this is something I have done in the past for my research. I am currently
working on a publication for 3DGS/4DGS, a real-time radiance field (equivalent of NeRF)
where obtaining a point cloud is straight forward.

To extract a mesh/point cloud from NeRF, one can use Poisson surface reconstruction to
extract points or a surface from the density field learned by NeRF. There are, however,
more advanced methods that can produce higher fidelity meshes than these traditional
approaches, e.g. [nerf2mesh](https://github.com/ashawkey/nerf2mesh).
