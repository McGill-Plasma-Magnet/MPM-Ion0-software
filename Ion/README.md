# Ion
Software for second Ion HAB flight, building off first flight software.

## Changes
* added Python GNSS module driver
* project build managed by uv
* main loop switched over to Python using OLAF (***TO DO***)
* GPS logs kept separately (***TO DO***)

## Installation and setup
Clone this repo, then go to the Ion directory.
```
git clone https://github.com/jacob-sauve/MPM-Ion0-software.git
cd MPM-Ion0-software/Ion
```
Run the following command to clean the repo:
`make clean`

### Run an example script
Assumes your example is within the Ion/src/ion/examples directory. Run:
`make run-ex ex=example_script_name.py`

### Run the main program
If you trust that your environment is not stale:
`make run`

Otherwise:
`make ion-fresh`
