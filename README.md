A scriptable interface for STL transformation and accumilation.


## Quickstart

```bash
$ git clone https://github.com/sshirokov/stltwalker.git
$ cd stltwalker
$ make
$ ./stltwalker -h
```

For a quick compositing demo try

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

Which should produece something like [this](http://cl.ly/image/3i1L2l39330H) when rendered or printed:

![stltwalker jaws composite](http://f.cl.ly/items/2J1U2F360Z3J2S0A0B1P/Image%202012.11.24%2011:51:35%20PM.png)
