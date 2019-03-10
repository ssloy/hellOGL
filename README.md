# hellOGL: modern OpenGL hello world

**Check [the wiki](https://github.com/ssloy/hellOGL/wiki) for a detailed description of the source code.**

The main idea of this repository is to show you how to create and initialize an OpenGL window; it is surprisingly difficult to have the very first code that compiles and works out of the box.
There are numerous repositories doing that, however (in my humble opinion) they miss the sweet spot: either they are hard to follow or the rendrings are ugly.

So here comes hellOGL:


![](https://raw.githubusercontent.com/ssloy/hellOGL/master/doc/99.png)

Let us thank Samuel (arshlevon) Sharit for this incredible model!


# Use CMake to build the project:

```sh
git clone --recurse-submodules https://github.com/ssloy/hellOGL.git
cd hellOGL
mkdir build
cd build
cmake ..
make
```

