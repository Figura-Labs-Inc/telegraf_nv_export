# nv_export

Ultra low overhead NVIDIA GPU telemetry plugin for telegraf with memory temperature readings.

## Building
requirements: CUDA, CMake, C++23 capable compiler, libpci (optional)
```sh
mkdir build
cd build
# build with  -DDRAM_TELEMETRY=NO if your gpu is not yet supported / can't run as root
cmake ..
make
# sudo cp nv_export /etc/telegraf/
```

Since telegraf doesn't run the executable as root, you need to set the capabilities of the executable to allow reading `/dev/mem`:
```sh
sudo setcap cap_sys_rawio,cap_dac_override+ep ./nv_export
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

# Notes on VRAM temperature readings

The hack is needed because calling `nvmlDeviceGetFieldValues()` with `NVML_FI_DEV_MEMORY_TEMP` returns error `NVML_ERROR_NOT_SUPPORTED`.

credits to [olealgoritme/gddr6](https://github.com/olealgoritme/gddr6)

## Prerequisites

- Kernel boot parameter: iomem=relaxed
```
sudo vim /etc/default/grub
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash iomem=relaxed"
sudo update-grub
sudo reboot
```

- Disabling Secure Boot

This can be done in the UEFI/BIOS configuration or using [mokutil](https://wiki.debian.org/SecureBoot#Disabling.2Fre-enabling_Secure_Boot):

```
mokutil --disable-validation
```

Check state with:
```
$ sudo mokutil --sb
SecureBoot disabled
```

## Dependencies
- libpci-dev
```
sudo apt install libpci-dev -y
```

## Supported GPUs
- RTX 4090 (AD102)
- RTX 4080 Super (AD103)
- RTX 4080 (AD103)
- RTX 4070 Ti Super (AD103)
- RTX 4070 Ti (AD104)
- RTX 4070 Super (AD104)
- RTX 4070 (AD104)
- RTX 3090 Ti (GA102)
- RTX 3090 (GA102)
- RTX 3080 Ti (GA102)
- RTX 3080 (GA102)
- RTX 3080 LHR (GA102)
- RTX 3070 (GA104)
- RTX 3070 LHR (GA104)
- RTX A2000 (GA106)
- RTX A4500 (GA102)
- RTX A5000 (GA102)
- RTX A6000 (AD102)
- L4 (AD104)
- L40 (AD102)
- L40S (AD102)
- A10 (GA102)