//
// Created by ant on 9/7/2020.
//

#ifndef OREO_LUNAR_UM_CONSTANTS_H
#define OREO_LUNAR_UM_CONSTANTS_H

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#include "driver.h"
#include "process.h"
#include "collection.h"
#include "log.h"
#include "settings.h"

size_t block_size = 0x800000;
size_t scan_size = 0x8000000000;

int threads = 0;

std::mutex thread_count_mutex;
std::mutex current_count_mutex;
std::mutex out_mutex;

class C_Constants {
private:
	uintptr_t ConstantPoolAddress{};
	std::vector<uintptr_t> ReachAddresses;

public:
	bool UpdateConstantPoolAddress()
	{
		if (!g_driver->bInitialized && !g_driver->Initialize())
			return false;

		if (!g_process->IsInitialized() && !g_process->Initialize())
			return false;

		ConstantPoolAddress = g_driver->get_module_base(g_process->dwProcessId, L"msvcr100.dll");

		std::cout << "Constant pool: " << ConstantPoolAddress << std::endl;

		return true;
	}

	static uint64_t* CopyRegion(uint64_t Base, uint64_t Size)
	{
		const auto Allocation = reinterpret_cast<uint64_t*>(malloc(Size));
		g_driver->copy_bytes(g_process->dwProcessId, Base, (uintptr_t)Allocation, Size);
		return Allocation;
	}

	static void ReachScan(uint64_t RegionAddress, uint64_t** RegionBase, uint64_t RegionSize)
	{
		//        double Values[] = {3.0, Settings::dReachDistance};
		//
		//        auto ScanResults = ScanRegionForValues<double>(
		//                RegionBase,
		//                RegionSize,
		//                Values);

		//        for (uint64_t *Address : ScanResults)
		//        {
		//            double SubValues[] = {4.5};
		//            auto SubSize = sizeof(double) * 21; // 10 before address, 10 after address. Includes address
		//            auto SubRegion = CopyRegion(RegionAddress + (Address - RegionBase) - SubSize / 2, SubSize);
		//
		//            auto SubScanResults = ScanRegionForValues(
		//                    SubRegion,
		//                    SubSize,
		//                    SubValues);
		//
		//            if (SubScanResults.empty())
		//                continue;
		//
		//            gConnection->write(gProcess->dwProcessId, RegionAddress + (Address - RegionBase), Settings::dReachDistance);
		//        }
	}

	template<class T, size_t N>
	static std::vector<uint64_t*> ScanRegionForValues(uint64_t* RegionBase, uint64_t RegionSize, T(&Values)[N])
	{
		std::vector<uint64_t*> MatchingAddresses;

		int iterations = 0;

		for (auto CurrentAddress = RegionBase; CurrentAddress < RegionBase + RegionSize; CurrentAddress++)
		{
			auto Value = reinterpret_cast<T*>(CurrentAddress);

			if (*Value == 3.0)
				std::cout << "OK" << std::endl;

			iterations++;
		}

		return MatchingAddresses;
	}

	static void thread_scan_region(uint64_t region_base_addy, uint64_t* alloc_base, int* current_count)
	{
		thread_count_mutex.lock();
		threads++;
		thread_count_mutex.unlock();

		for (uint64_t* alloc_addy = alloc_base; uint64_t(alloc_addy) < uint64_t(alloc_base) + block_size; alloc_addy++)
		{
			double value = *reinterpret_cast<double*>(alloc_addy);

			if (value == 3.0)
			{
				out_mutex.lock();
				g_driver->write<double>(g_process->dwProcessId,
					uint64_t(region_base_addy) + (uint64_t(alloc_addy) - uint64_t(alloc_base)), 4.0);
				out_mutex.unlock();

				uint64_t sub_region_size = sizeof(double) * 50;

				auto* sub_alloc_base = reinterpret_cast<uint64_t*>(malloc(sub_region_size));
				g_driver->copy_bytes(g_process->dwProcessId,
					int64_t(region_base_addy) + (uint64_t(alloc_addy) - uint64_t(alloc_base)) -
					(sub_region_size / 2),
					uint64_t(sub_alloc_base), sub_region_size);

				for (uint64_t* sub_alloc_addy = sub_alloc_base; uint64_t(sub_alloc_addy) < uint64_t(sub_alloc_base) + sub_region_size; sub_alloc_addy += 1)
				{
					double val = *reinterpret_cast<double*>(sub_alloc_addy);

					if (val == 4.5)
					{
						out_mutex.lock();
						std::cout << "found possible reach address: " << uint64_t(region_base_addy) + (uint64_t(alloc_addy) - uint64_t(alloc_base)) << std::endl;
						out_mutex.unlock();

						g_driver->write<double>(g_process->dwProcessId, uint64_t(region_base_addy) +
							(uint64_t(alloc_addy) - uint64_t(alloc_base)),
							4.0);
					}
				}

				free(sub_alloc_base);
			}
		}

		thread_count_mutex.lock();
		threads--;
		thread_count_mutex.unlock();

		current_count_mutex.lock();
		(*current_count)++;
		current_count_mutex.unlock();

		free(alloc_base);
	}

	void ScanConstantPool() const
	{
		DWORD CurrentTickCount = GetTickCount();
		int current_count = 0;

		float last_logged_percent = 0.0f;
		float total_blocks = scan_size / block_size;

		for (auto address = ConstantPoolAddress; address < ConstantPoolAddress + scan_size; address += block_size)
		{
			float current_percent = (current_count / total_blocks) * 100;
			if (current_percent - last_logged_percent >= 1.0f || current_percent >= 100)
			{
				std::cout << "Scanning regions: " << current_percent << "%; "
					<< current_count << " out of " << total_blocks << " blocks ("
					<< (int)block_size / 1000 / 1000 << "MB each); "
					<< "Time elapsed: " << (GetTickCount() - CurrentTickCount) / 1000 << "s; "
					<< "Threads: " << threads
					<< std::endl;
				last_logged_percent = current_percent;
			}

			// Allocate memory to copy into
			auto alloc_base = reinterpret_cast<uint64_t*>(malloc(block_size));

			// Copy process memory to allocation
			g_driver->copy_bytes(g_process->dwProcessId, address, reinterpret_cast<uint64_t>(alloc_base), block_size);

			// Run thread to scan the region
			std::thread(thread_scan_region, address, alloc_base, &current_count).detach();
		}

		while (threads > 0)
			Sleep(5);

		DWORD scanTime = GetTickCount() - CurrentTickCount;
		std::cout << "Finished scan (Time: " << scanTime / 1000 << "s)" << std::endl;
	}
};

C_Constants* gConstants = new C_Constants();

#endif //OREO_LUNAR_UM_CONSTANTS_H
