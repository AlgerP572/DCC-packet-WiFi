#include <WifiSerialDebug.h>

#include "CpuUsage.h"
#include "esp_freertos_hooks.h"

static volatile uint64_t idle0Calls = 0;
static volatile uint64_t idle1Calls = 0;

#define CONFIG_ESP32_DEFAULT_CPU_FREQ_240

#if defined(CONFIG_ESP32_DEFAULT_CPU_FREQ_240)
static const uint64_t MaxIdleCalls = 1855000;
#elif defined(CONFIG_ESP32_DEFAULT_CPU_FREQ_160)
static const uint64_t MaxIdleCalls = 1233100;
#else
#error "Unsupported CPU frequency"
#endif

float CpuUsage::_cpuLoad = 0;


static bool idle_task_0()
{
	idle0Calls += 1;
	return false;
}

static bool idle_task_1()
{
	idle1Calls += 1;
	return false;
}

void CpuUsage::setup()
{
    ESP_ERROR_CHECK(esp_register_freertos_idle_hook_for_cpu(idle_task_0, 0));
	ESP_ERROR_CHECK(esp_register_freertos_idle_hook_for_cpu(idle_task_1, 1));
	
	xTaskCreate(measureTask,
        "perfmon",
        2048,
        NULL,
        10,
        NULL);
	return;
}

void CpuUsage::measureTask(void *args)
{
	while (true)
	{
		uint64_t idle0 = idle0Calls;
		uint64_t idle1 = idle1Calls;
		idle0Calls = 0;
		idle1Calls = 0;

		float cpu0 = 100.f -  static_cast<float>(idle0) / MaxIdleCalls * 100.f;
		float cpu1 = 100.f - static_cast<float>(idle1) / MaxIdleCalls * 100.f;

        // Note: in ESP32 Arduino core 0 runs WiFi and bluetooth
        // tasks so may never reach 0 per cent cpu.
        cpu0 = std::min(cpu0, 100.f);
        cpu1 = std::min(cpu1, 100.f);
        cpu0 = std::max(cpu0, 0.f);        
        cpu1 = std::max(cpu1, 0.f);
        
        _cpuLoad = (cpu0 + cpu1) / 2.0f;

        // Useful to look at how aogothm works.
        // Log::TakeMultiPrintSection();
        // Log::print("Total Cpu: ", LogLevel::DEBUG);
        // Log::print(_cpuLoad, LogLevel::DEBUG);        
        // Log::GiveMultiPrintSection();		
		
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

float CpuUsage::GetCpuLoad()
{
    return _cpuLoad;
}