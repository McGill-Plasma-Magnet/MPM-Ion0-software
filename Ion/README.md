# Ion
Software for second Ion HAB flight, building off first flight software.

## Changes
* added Python GNSS module driver
* project build managed by uv
* main loop switched over to Python using OLAF
* GPS logs kept separately (***TO DO***)

## Installation and setup
Go to the Ion directory.

To set up the environment and run the program:
```
make run
```

### Run an example script
Assumes your example is within the examples/ directory. Run:
```
make run-ex ex=example_script_name
```

### Run the main program
If you trust that your environment is not stale:
```
make run
```

Otherwise, to rebuild from scratch and run:
```
make ion-fresh
```
