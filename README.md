A scriptable interface for STL transformation and accumilation.

## Quickstart

```bash
$ git clone https://github.com/sshirokov/stltwalker.git
$ cd stltwalker
$ make
$ ./stltwalker -h
./stltwalker [-options] {file0 [--operations]..}..{fileN [--operations]..}
	v0.0.3

Options:
	-h	Show help
	-I	Copy maximum 1 input object to output memory directly.
	-L <float>	Maximum result object length
	-W <float>	Maximum result object width
	-H <float>	Maximum result object height
	-p	Pack input objects automatically
	-b <float>	Set packing margin
	-o filename	Output the resulting composite object to `filename'
	-r	Do not center and raise object above the Z plane on load
	-R	Do not center and raise the result object above the Z plane
	-D	Increase the detail with which the result is described

Transforms:
	--scale=<options>	Scale the model by a constant factor
	--rotateX=<options>	Rotate the model around the X axis (degrees)
	--rotateY=<options>	Rotate the model around the Y axis (degrees)
	--rotateZ=<options>	Rotate the model around the Z axis (degrees)
	--translate=<options>	Translate the model along a vector <X,Y,Z>
```

`-L`, `-W`, and `-H` will change the exit status of the application to non-zero as well as printing the failure.


## Eyedropping Demo With Special Effects

### Packing

The `-p` option asks stltwalker to pack the input objects on the build platform.

The packing margin can be adjusted with `-b 25.0`

```bash
$ ./stltwalker -p data/jaws.stl --rotateZ=45.0 \
                  data/jaws.stl                \
                  data/jaws.stl                \
                  -o /tmp/packed.stl
```

Which should result in something like this:

![stltwalker packing composite](http://cl.ly/image/3L2p13000R32/Image%202012.12.18%201:49:05%20AM.png)

### Compositing

By default, the inputs are transformed and added to the build platform.

```bash
$ ./stltwalker data/jaws.stl --rotateZ=10                      \
               data/jaws.stl --translate=0,30,0                \
               data/jaws.stl --translate=0,-30,0 --rotateZ=180 \
               -o /tmp/out.stl

[INFO] (src/stltwalker.c:114) Loaded: data/jaws.stl
[INFO] (src/stltwalker.c:114) Loaded: data/jaws.stl
[INFO] (src/stltwalker.c:114) Loaded: data/jaws.stl
[INFO] (src/stltwalker.c:147) Output contains 840 facets
[INFO] (src/stltwalker.c:152) Writing result object to: '/tmp/out.stl'
```

Which should produce something like [this](http://cl.ly/image/3i1L2l39330H) when rendered or printed:

![stltwalker jaws composite](http://f.cl.ly/items/2J1U2F360Z3J2S0A0B1P/Image%202012.11.24%2011:51:35%20PM.png)
