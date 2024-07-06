#include <iostream>
#include <chrono>
#include <expected>
#include <thread>
#include <unordered_map>
#include <cstdlib>
#include <format>

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>

#ifndef NO_DRAM_TELEMETRY
extern "C" {
    #include <pci/pci.h>
}
#endif

#include <reflect>
#include <range/v3/all.hpp>
#include <nvml.h>


template <typename T>
using nvField = std::expected<T, nvmlReturn_t>;

template <typename T>
auto nvCheckCall(nvField<T>& res, nvmlReturn_t val) -> bool {
    if (val != NVML_SUCCESS) {
        res = std::unexpected(val);
        return false;
    }
    return true;
}

struct nvml_gpu_data
{
    nvField<uint32_t> tag_index;
    nvField<std::string> tag_name;
    nvField<std::string> tag_bus_id;
    nvField<std::string> tag_uuid;
    nvField<uint16_t> dev_id;
    nvField<uint32_t> pcie_link_gen_current;
    nvField<uint32_t> pcie_link_gen_max;
    nvField<uint32_t> pcie_link_width_current;
    nvField<uint32_t> pcie_link_width_max;
    nvField<uint32_t> persistence_mode;
    nvField<uint32_t> fan_speed;
    nvField<uint32_t> pstate;
    nvField<bool> clock_throttle_reason_sw_power_cap;
    nvField<bool> clock_throttle_reason_hw_slowdown;
    nvField<bool> clock_throttle_reason_hw_thermal_slowdown;
    nvField<bool> clock_throttle_reason_hw_power_brake_slowdown;
    nvField<bool> clock_throttle_reason_sw_thermal_slowdown;
    nvField<bool> clock_throttle_reason_sync_boost;
    nvField<uint64_t> memory_total;
    nvField<uint64_t> memory_used;
    nvField<std::string> compute_cap;
    nvField<float> utilization_gpu;
    nvField<float> utilization_memory;
    nvField<uint32_t> temperature_gpu;
    nvField<uint32_t> power_draw;
    nvField<uint32_t> power_limit;
    nvField<uint32_t> clocks_gr;
    nvField<uint32_t> clocks_sm;
    nvField<uint32_t> clocks_mem;
    nvField<uint32_t> clocks_video;

    static nvml_gpu_data from_handle(nvmlDevice_t device) {
        nvml_gpu_data data;

        char name[NVML_DEVICE_NAME_BUFFER_SIZE];
        if (nvCheckCall(data.tag_name, nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE))) {
            data.tag_name = name;
        }
        nvmlPciInfo_t pci_info;
        if (nvCheckCall(data.tag_bus_id, nvmlDeviceGetPciInfo_v3(device, &pci_info))) {
            data.tag_bus_id = pci_info.busId;
            data.dev_id = pci_info.pciDeviceId;
        }
        char uuid[NVML_DEVICE_UUID_V2_BUFFER_SIZE];
        if (nvCheckCall(data.tag_uuid, nvmlDeviceGetUUID(device, uuid, NVML_DEVICE_UUID_V2_BUFFER_SIZE))) {
            data.tag_uuid = uuid;
        }
        uint32_t pcie_link_gen_current;
        if (nvCheckCall(data.pcie_link_gen_current, nvmlDeviceGetCurrPcieLinkGeneration(device, &pcie_link_gen_current))) {
            data.pcie_link_gen_current = pcie_link_gen_current;
        }
        uint32_t pcie_link_gen_max;
        if (nvCheckCall(data.pcie_link_gen_max, nvmlDeviceGetMaxPcieLinkGeneration(device, &pcie_link_gen_max))) {
            data.pcie_link_gen_max = pcie_link_gen_max;
        }
        uint32_t pcie_link_width_current;
        if (nvCheckCall(data.pcie_link_width_current, nvmlDeviceGetCurrPcieLinkWidth(device, &pcie_link_width_current))) {
            data.pcie_link_width_current = pcie_link_width_current;
        }
        uint32_t pcie_link_width_max;
        if (nvCheckCall(data.pcie_link_width_max, nvmlDeviceGetMaxPcieLinkWidth(device, &pcie_link_width_max))) {
            data.pcie_link_width_max = pcie_link_width_max;
        }
        nvmlEnableState_t persistence_mode;
        if (nvCheckCall(data.persistence_mode, nvmlDeviceGetPersistenceMode(device, &persistence_mode))) {
            data.persistence_mode = (persistence_mode == NVML_FEATURE_ENABLED);
        }
        uint32_t fan_speed;
        if (nvCheckCall(data.fan_speed, nvmlDeviceGetFanSpeed(device, &fan_speed))) {
            data.fan_speed = fan_speed;
        }
        nvmlPstates_t pstate;
        if (nvCheckCall(data.pstate, nvmlDeviceGetPerformanceState(device, &pstate))) {
            data.pstate = (int)pstate;
        }
        unsigned long long clock_event_reasons;
        if (nvCheckCall(data.clock_throttle_reason_sw_power_cap, nvmlDeviceGetCurrentClocksEventReasons(device, &clock_event_reasons))) {
            data.clock_throttle_reason_sw_power_cap = (clock_event_reasons & nvmlClocksThrottleReasonSwPowerCap) != 0;
            data.clock_throttle_reason_hw_slowdown = (clock_event_reasons & nvmlClocksThrottleReasonHwSlowdown) != 0;
            data.clock_throttle_reason_hw_thermal_slowdown = (clock_event_reasons & nvmlClocksThrottleReasonHwThermalSlowdown) != 0;
            data.clock_throttle_reason_hw_power_brake_slowdown = (clock_event_reasons & nvmlClocksThrottleReasonHwPowerBrakeSlowdown) != 0;
            data.clock_throttle_reason_sw_thermal_slowdown = (clock_event_reasons & nvmlClocksThrottleReasonSwThermalSlowdown) != 0;
            data.clock_throttle_reason_sync_boost = (clock_event_reasons & nvmlClocksThrottleReasonSyncBoost) != 0;
        }
        nvmlMemory_t memory;
        if (nvCheckCall(data.memory_total, nvmlDeviceGetMemoryInfo(device, &memory))) {
            data.memory_total = memory.total;
            data.memory_used = memory.used;
        }
        int32_t major, minor;
        if (nvCheckCall(data.compute_cap, nvmlDeviceGetCudaComputeCapability(device, &major, &minor))) {
            data.compute_cap = std::to_string(major) + "." + std::to_string(minor);
        }
        nvmlUtilization_t utilization;
        if (nvCheckCall(data.utilization_gpu, nvmlDeviceGetUtilizationRates(device, &utilization))) {
            data.utilization_gpu = utilization.gpu;
            data.utilization_memory = utilization.memory;
        }
        uint32_t temperature_gpu;
        if (nvCheckCall(data.temperature_gpu, nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature_gpu))) {
            data.temperature_gpu = temperature_gpu;
        }
        uint32_t power;
        if (nvCheckCall(data.power_draw, nvmlDeviceGetPowerUsage(device, &power))) {
            data.power_draw = power;
        }
        uint32_t power_limit;
        if (nvCheckCall(data.power_limit, nvmlDeviceGetPowerManagementLimit(device, &power_limit))) {
            data.power_limit = power_limit;
        }
        uint32_t clock_gr;
        if (nvCheckCall(data.clocks_gr, nvmlDeviceGetClockInfo(device, NVML_CLOCK_GRAPHICS, &clock_gr))) {
            data.clocks_gr = clock_gr;
        }
        uint32_t clock_sm;        
        if (nvCheckCall(data.clocks_sm, nvmlDeviceGetClockInfo(device, NVML_CLOCK_SM, &clock_sm))) {
            data.clocks_sm = clock_sm;
        }
        uint32_t clock_mem;
        if (nvCheckCall(data.clocks_mem, nvmlDeviceGetClockInfo(device, NVML_CLOCK_MEM, &clock_mem))) {
            data.clocks_mem = clock_mem;
        }
        uint32_t clock_video;
        if (nvCheckCall(data.clocks_video, nvmlDeviceGetClockInfo(device, NVML_CLOCK_VIDEO, &clock_video))) {
            data.clocks_video = clock_video;
        }

        return data;
    }
};

struct dev_table_entry
{
    uint32_t offset;
    const char *vram;
    const char *arch;
    const char *name;
};

static std::unordered_map<uint16_t, dev_table_entry> dev_table_map = {
    // Ada
    { 0x2684, { .offset = 0x0000E2A8, .vram = "GDDR6X", .arch = "AD102", .name =  "RTX 4090" } },
    { 0x2702, { .offset = 0x0000E2A8, .vram = "GDDR6X", .arch = "AD103", .name =  "RTX 4080 Super" } },
    { 0x2704, { .offset = 0x0000E2A8, .vram = "GDDR6X", .arch = "AD103", .name =  "RTX 4080" } },
    { 0x2705, { .offset = 0x0000E2A8, .vram = "GDDR6X", .arch = "AD103", .name =  "RTX 4070 Ti Super" } },
    { 0x2782, { .offset = 0x0000E2A8, .vram = "GDDR6X", .arch = "AD104", .name =  "RTX 4070 Ti" } },
    { 0x2783, { .offset = 0x0000E2A8, .vram = "GDDR6X", .arch = "AD104", .name =  "RTX 4070 Super" } },
    { 0x2786, { .offset = 0x0000E2A8, .vram = "GDDR6X", .arch = "AD104", .name =  "RTX 4070" } },
    { 0x2860, { .offset = 0x0000E2A8, .vram = "GDDR6",  .arch = "AD106", .name =  "RTX 4070 Max-Q / Mobile" } },
    { 0x26B1, { .offset = 0x0000E2A8, .vram = "GDDR6",  .arch = "AD102", .name =  "RTX A6000 (Ada)" } },
    { 0x27b8, { .offset = 0x0000E2A8, .vram = "GDDR6",  .arch = "AD104", .name =  "L4" } },
    { 0x26B9, { .offset = 0x0000E2A8, .vram = "GDDR6",  .arch = "AD102", .name =  "L40S" } },
    { 0x26B5, { .offset = 0x0000E2A8, .vram = "GDDR6",  .arch = "AD102", .name =  "L40" } },
    // Ampere
    { 0x2203, { .offset = 0x0000E2A8, .vram = "GDDR6X", .arch = "GA102", .name =  "RTX 3090 Ti" } },
    { 0x2204, { .offset = 0x0000E2A8, .vram = "GDDR6X", .arch = "GA102", .name =  "RTX 3090" } },
    { 0x2208, { .offset = 0x0000E2A8, .vram = "GDDR6X", .arch = "GA102", .name =  "RTX 3080 Ti" } },
    { 0x2206, { .offset = 0x0000E2A8, .vram = "GDDR6X", .arch = "GA102", .name =  "RTX 3080" } },
    { 0x2216, { .offset = 0x0000E2A8, .vram = "GDDR6X", .arch = "GA102", .name =  "RTX 3080 LHR" } },
    { 0x2484, { .offset = 0x0000EE50, .vram = "GDDR6",  .arch = "GA104", .name =  "RTX 3070" } },
    { 0x2488, { .offset = 0x0000EE50, .vram = "GDDR6",  .arch = "GA104", .name =  "RTX 3070 LHR" } },
    { 0x2531, { .offset = 0x0000E2A8, .vram = "GDDR6",  .arch = "GA106", .name =  "RTX A2000" } },
    { 0x2571, { .offset = 0x0000E2A8, .vram = "GDDR6",  .arch = "GA106", .name =  "RTX A2000" } },
    { 0x2232, { .offset = 0x0000E2A8, .vram = "GDDR6",  .arch = "GA102", .name =  "RTX A4500" } },
    { 0x2231, { .offset = 0x0000E2A8, .vram = "GDDR6",  .arch = "GA102", .name =  "RTX A5000" } },
    { 0x2236, { .offset = 0x0000E2A8, .vram = "GDDR6",  .arch = "GA102", .name =  "A10" } },
};


struct device
{
    uint32_t bar0;
    uint8_t bus, dev, func;
    nvmlPciInfo_t pci_info;
    dev_table_entry meta;
    void *mapped_addr;
    uint32_t phys_addr;
    uint32_t base_offset;
    nvmlDevice_t nvml_device;
};

auto escape_tag(std::string_view s) -> std::string {
    auto escape_char = [](char c) -> std::string {
        switch (c) {
            case ',': return {"\\,"};
            case '=': return {"\\="};
            case ' ': return {"\\ "};
            default: return {c};
        }
    };

    return s
        | ranges::views::transform(escape_char)
        | ranges::views::join
        | ranges::to<std::string>();
}

auto escape_string(std::string_view s) -> std::string {
    auto escape_char = [](char c) -> std::string{
        switch (c) {
            case '"': return {'\\', '"'};
            case '\\': return {'\\', '\\'};
            default: return {c};
        }
    };

    return s
        | ranges::views::transform(escape_char)
        | ranges::views::join
        | ranges::to<std::string>();
};

int main() {
    nvmlReturn_t result;

    result = nvmlInit();
    if (result != NVML_SUCCESS) {
        std::cerr << "Failed to initialize NVML: " << nvmlErrorString(result) << std::endl;
        return 1;
    }

    std::atexit([](){
        nvmlShutdown();
    });

    uint32_t device_count;
    result = nvmlDeviceGetCount(&device_count);
    if (result != NVML_SUCCESS) {
        std::cerr << "Failed to get device count: " << nvmlErrorString(result) << std::endl;
        return 1;
    }

    static std::vector<device> devices(device_count);
    for (uint32_t i = 0; i < device_count; ++i) {
        result = nvmlDeviceGetHandleByIndex(i, &devices[i].nvml_device);
        if (result != NVML_SUCCESS) {
            std::cerr << "Failed to get handle for device " << i << ": " << nvmlErrorString(result) << std::endl;
            return 1;
        }

        nvmlPciInfo_t pci_info;
        if (nvmlDeviceGetPciInfo_v3(devices[i].nvml_device, &pci_info) != NVML_SUCCESS) {
            std::cerr << "Failed to get Pci info for device " << i << ": " << nvmlErrorString(result) << std::endl;
            return 1;
        }
        devices[i].pci_info = pci_info;
    }

#ifndef NO_DRAM_TELEMETRY
    static int pg_sz = sysconf(_SC_PAGE_SIZE);
    static int memfd = open("/dev/mem", O_RDONLY);
    if (memfd == -1) {
        std::cerr << "Failed to open /dev/mem, are you root?" << std::endl;
        return 1;
    }

    pci_access *pacc = NULL;
    pci_dev *pci_dev = NULL;

    pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);

    for (pci_dev = pacc->devices; pci_dev != NULL; pci_dev = pci_dev->next) {
        pci_fill_info(pci_dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);
        auto it = dev_table_map.find(pci_dev->device_id);
        if (it == dev_table_map.end())
            continue;

        for (auto& device : devices) {
            if (device.pci_info.bus == pci_dev->bus
                && device.pci_info.device == pci_dev->dev
                && device.pci_info.domain == pci_dev->func) {
                device.meta = it->second;
                device.bar0 = (pci_dev->base_addr[0] & 0xffffffff);
                device.bus = pci_dev->bus;
                device.dev = pci_dev->dev;
                device.func = pci_dev->func;
                break;
            }
        }
    }

    for (auto& device : devices) {
        if (device.meta.vram == NULL) {
            std::cerr << "No matching device found for " << device.pci_info.bus << ":" << device.pci_info.device << ":" << device.pci_info.domain << std::endl
                << "Please add a matching entry to dev_table_map in nv_export.cpp or compile with NO_DRAM_TELEMETRY to disable this check." << std::endl;
            return 1;
        }

        device.phys_addr = (device.bar0 + device.meta.offset);
        device.base_offset = device.phys_addr & ~(pg_sz - 1);

        device.mapped_addr = mmap(0, pg_sz, PROT_READ, MAP_SHARED, memfd, device.base_offset);
        if (device.mapped_addr == MAP_FAILED)
        {
            device.mapped_addr = NULL;
            std::cerr << std::format("Memory mapping failed for pci={:x}:{:x}:{:x}\n", device.pci_info.bus, device.pci_info.device, device.pci_info.domain);
            std::cerr << "Did you enable iomem=relaxed? Are you root?\n";
            return 1;
        }
    }

    atexit([](){
        for (auto& device : devices) {
            if (device.mapped_addr != NULL && device.mapped_addr != MAP_FAILED) {
                munmap(device.mapped_addr, pg_sz);
                device.mapped_addr = NULL;
            }
        }
        if (memfd != -1) {
            close(memfd);
            memfd = -1;
        }
    });
#endif


    while (true) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count();

        for (auto const [idx, device] : ranges::views::enumerate(devices)) {
            nvml_gpu_data gpu = nvml_gpu_data::from_handle(device.nvml_device);
            gpu.tag_index = idx;

            std::vector<std::string> metrics;
            std::vector<std::string> errors;

            reflect::for_each([&](auto I) {
                auto field = reflect::get<I>(gpu);
                std::string_view name = reflect::member_name<I>(gpu);
                if (name.starts_with("tag_")) return;

                if (field) {
                    using field_T = std::remove_cvref_t<decltype(*field)>;
                    std::string value;
                    if      constexpr (std::is_same_v<field_T, std::string>) value = std::format("\"{}\"", escape_string(*field));
                    else if constexpr (std::is_same_v<field_T, bool>)        value = (*field ? "true" : "false");
                    else if constexpr (std::is_same_v<field_T, uint64_t>)    value = std::format("{}u", *field);
                    else                                                     value = std::format("{}", *field);

                    metrics.push_back(std::format("{}={}", name, value));
                } else {
                    errors.push_back(std::format("{} ({})", name, reflect::enum_name(field.error())));
                }
            }, gpu);

#ifndef NO_DRAM_TELEMETRY
            if (device.mapped_addr != NULL && device.mapped_addr != MAP_FAILED) {
                void *virt_addr = (uint8_t *)device.mapped_addr + (device.phys_addr  - device.base_offset);
                uint32_t read_result = *((uint32_t *)virt_addr);
                uint32_t temp = ((read_result & 0x00000fff) / 0x20);
                metrics.push_back(std::format("temperature_memory={}", temp));
            }
#endif

            if (!errors.empty()) {
                auto error_str = errors
                    | ranges::views::join(",")
                    | ranges::to<std::string>();
                metrics.push_back(std::format("errors=\"{}\"", escape_string(error_str)));
            }

            std::cout << "nv_export"
                << ",index=" << *gpu.tag_index
                << ",name=" << escape_tag(*gpu.tag_name)
                << ",bus_id=" << escape_tag(*gpu.tag_bus_id)
                << ",uuid=" << escape_tag(*gpu.tag_uuid)
                << " " << (metrics | ranges::views::join(",") | ranges::to<std::string>())
                << " " << timestamp << std::endl;
        }
        std::flush(std::cout);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
