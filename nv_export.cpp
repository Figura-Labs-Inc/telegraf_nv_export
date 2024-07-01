#include <iostream>
#include <chrono>
#include <sstream>
#include <ranges>
#include <expected>
#include <thread>

#include <reflect>
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

struct gpu_data
{
    nvField<uint32_t> index;
    nvField<std::string> name;
    nvField<std::string> bus_id;
    nvField<std::string> uuid;
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
    nvField<uint32_t> temperature_memory;
    nvField<uint32_t> power_draw;
    nvField<uint32_t> power_limit;
    nvField<uint32_t> clocks_gr;
    nvField<uint32_t> clocks_sm;
    nvField<uint32_t> clocks_mem;
    nvField<uint32_t> clocks_video;

    static gpu_data from_handle(nvmlDevice_t device) {
        gpu_data data;

        char name[NVML_DEVICE_NAME_BUFFER_SIZE];
        if (nvCheckCall(data.name, nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE))) {
            data.name = name;
        }
        nvmlPciInfo_t pci_info;
        if (nvCheckCall(data.bus_id, nvmlDeviceGetPciInfo_v3(device, &pci_info))) {
            data.bus_id = pci_info.busId;
        }
        char uuid[NVML_DEVICE_UUID_V2_BUFFER_SIZE];
        if (nvCheckCall(data.uuid, nvmlDeviceGetUUID(device, uuid, NVML_DEVICE_UUID_V2_BUFFER_SIZE))) {
            data.uuid = uuid;
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
        uint32_t temperature_memory;
        if (nvCheckCall(data.temperature_memory, nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature_memory))) {
            data.temperature_memory = temperature_memory;
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


int main() {
    nvmlReturn_t result;

    result = nvmlInit();
    if (result != NVML_SUCCESS) {
        std::cerr << "Failed to initialize NVML: " << nvmlErrorString(result) << std::endl;
        return 1;
    }

    uint32_t device_count;
    result = nvmlDeviceGetCount(&device_count);
    if (result != NVML_SUCCESS) {
        std::cerr << "Failed to get device count: " << nvmlErrorString(result) << std::endl;
        nvmlShutdown();
        return 1;
    }

    std::vector<nvmlDevice_t> devices(device_count, nullptr);
    for (uint32_t i = 0; i < device_count; ++i) {
        result = nvmlDeviceGetHandleByIndex(i, &devices[i]);
        if (result != NVML_SUCCESS) {
            std::cerr << "Failed to get handle for device " << i << ": " << nvmlErrorString(result) << std::endl;
            nvmlShutdown();
            return 1;
        }
    }

    while (true) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count();

        for (auto const [idx, device] : std::views::enumerate(devices)) {
            gpu_data gpu_info = gpu_data::from_handle(device);
            gpu_info.index = idx;

            std::stringstream ss;
            std::stringstream errors;
            bool first = true;
            ss << "nv_export ";
            reflect::for_each([&](auto I) {
                auto field = reflect::get<I>(gpu_info);
                if (field.has_value()) {
                    if (!std::exchange(first, false)) {
                        ss << ",";
                    }
                    ss << reflect::member_name<I>(gpu_info) << "=";
                    if constexpr (std::is_same_v<std::remove_cvref_t<decltype(*field)>, std::string>) {
                        ss << "\"" << *field << "\"";
                    } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(*field)>, bool>) {
                        ss << (*field ? "true" : "false");
                    } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(*field)>, uint64_t>) {
                        ss << *field << "u";
                    } else {
                        ss << *field;
                    }
                } else {
                    errors << reflect::member_name<I>(gpu_info) << " (" << reflect::enum_name(field.error()) << ");";
                }
            }, gpu_info);

            if (!errors.str().empty()) {
                ss << ",errors=\"" << errors.str() << "\"";
            }
            ss << " " << timestamp;
            std::cout << ss.str() << std::endl;
        }
        std::flush(std::cout);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}