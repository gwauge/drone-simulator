# Drone simulator written in C

## How to run
### Using pre-built binary
- Download latest release binary from GitHub
- make executable
```bash
chmod +x drone-simulator_linux_amd64
```
- run binary

### Compiling source
#### Requirements
- `gcc` $\geq$ 12.3.0
- `cmake` $\geq$ 3.10

#### Building
```bash
mkdir build
cd build/
cmake ..
make
```

#### Run
```bash
./dronesim
```

#### Debugging
In order to enable debugging symbols, run `cmake` with the corresponding flag.
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```
