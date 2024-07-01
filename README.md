# nv_export

Ultra low overhead NVIDIA GPU telemetry plugin for telegraf.

## Building
```sh
mkdir build
cd build
cmake ..
make
# cp ./build/nv_export /etc/telegraf/
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