# nv_export

Ultra low overhead NVIDIA GPU telemetry plugin for telegraf.

## Building
requirements: CUDA, CMake, C++23 capable compiler
```sh
mkdir build
cd build
cmake ..
make
# sudo cp nv_export /etc/telegraf/
```

## Telegraf Configuration
```toml
# ...
[[inputs.execd]]
    command = ["/etc/telegraf/nv_export"]
    data_format = "influx"
    signal = "none"
# ...
```