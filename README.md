# dumpStdHep
Show particles in .stdhep or .slcio (particles format) file.
This util works with CEPC soft.


## Build

1. Setup CEPC soft chain


2. Run

```
cd path/to/dumpStdHep
mkdir build
cd build
cmake path/to/dumpStdHep
make install
```

3. You could find the binary at `path/to/dumpStdHep/bin`.

## Run
```
path/to/dumpStdHep/bin/dumpStdHep [options] <stdhep/file> <event_number(starts width 0)>
```

use option `--help` to see more options
