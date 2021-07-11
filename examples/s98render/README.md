# s98render
Generate a wav file from a S98 file.

## Build
```
clang++ --std=c++17 -liconv -I../../src s98render.cpp s98file.cpp ../../src/ymfm_misc.cpp ../../src/ymfm_opl.cpp ../../src/ymfm_opm.cpp ../../src/ymfm_opn.cpp ../../src/ymfm_adpcm.cpp ../../src/ymfm_pcm.cpp ../../src/ymfm_ssg.cpp -o s98render
```

## Usage
```
s98render <inputfile> -o <outputfile> [-v <ssg volume ratio>] [-l <loop count>] [-r <rate>]
```

## Tips
When playing PC-98 sound log, the ssg volume ratio should be set to ~0.25.

## Limitations
- I have confirmed that I can build it on a Mac (Big Sur).
- I have only confirmed that it works with some files for OPNA.