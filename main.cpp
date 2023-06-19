#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cctype>

inline std::vector<std::uint8_t> AoB(const char* pattern) {
	std::vector<std::uint8_t> bytes{};

	auto start = const_cast<char*>(pattern);
	auto end = const_cast<char*>(pattern) + strlen(pattern);

	for (auto current = start; current < end; ++current) {
		if (*current == ' ') {
			continue;
		}

		bytes.push_back(strtoul(current, &current, 16));
	}

	return bytes;
}

inline uintptr_t FindPattern(HMODULE m, std::string_view pattern) {
	auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(m);
	auto nt = reinterpret_cast<PIMAGE_NT_HEADERS64>(reinterpret_cast<uintptr_t>(m) + dos->e_lfanew);

	auto size = nt->OptionalHeader.SizeOfImage;
	auto bytes = AoB(pattern.data());
	auto length = bytes.size();

	for (uintptr_t i = 0; i < size - length; i++) {
		bool found = true;

		for (uintptr_t j = 0; j < length; j++) {
			if (bytes[j] != *reinterpret_cast<uint8_t*>((uintptr_t)m + i + j) && bytes[j] != 0x00) {
				found = false;
				break;
			}
		}

		if (found) {
			return (uintptr_t)m + i;
		}
	}

	return 0;
}

int Sum(int a, int b)
{
	return a + b;
}

int hookedSum(int a, int b) {
	std::cout << "Hooked!\n";
	return 0;
}

uintptr_t sumOriginal = 0;

int main()
{
	std::cout << "Hello World!\n";

	HMODULE igo = LoadLibraryA("IGO64.dll");
	if (!igo)
	{
		std::cout << "Failed to load IGO64.dll\n";
		return 1;
	}
	std::cout << "IGO.dll loaded\n";

	uintptr_t createHookAddr = FindPattern(igo, "40 55 56 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24");
	uintptr_t removeHookAddr = FindPattern(igo, "48 89 54 24 ? 53 48 83 EC 30 48 8B D9");

	if (!createHookAddr)
	{
		std::cout << "Failed to find CreateHook\n";
		return 1;
	}
	std::cout << "CreateHook: " << std::hex << createHookAddr << "\n";
	std::cout << "offset: " << std::hex << (createHookAddr - (uintptr_t)igo) << "\n";

	if (!removeHookAddr)
	{
		std::cout << "Failed to find RemoveHookAddr\n";
		return 1;
	}
	std::cout << "RemoveHookAddr: " << std::hex << removeHookAddr << "\n";
	std::cout << "offset: " << std::hex << (removeHookAddr - (uintptr_t)igo) << "\n";

	auto CreateHook = reinterpret_cast<bool(*)(uintptr_t, uintptr_t, uintptr_t*, char)>(createHookAddr);
	auto RemoveHook = reinterpret_cast<bool (*)(uintptr_t*)>(removeHookAddr);

	bool result = CreateHook((uintptr_t)Sum, reinterpret_cast<uintptr_t>(hookedSum), (uintptr_t*)(&sumOriginal), 0);
	std::cout << "CreateHook result: " << std::hex << result << "\n";

	while (!GetAsyncKeyState(VK_SPACE))
	{
		std::cout << "Sum(1, 2) = " << Sum(1, 2) << "\n";
		Sleep(1000);
	}

	result = (bool)RemoveHook(reinterpret_cast<uintptr_t*>(&sumOriginal));
	std::cout << "RemoveHook result: " << std::hex << result << "\n";

	std::cout << "Sum(1, 2) = " << Sum(1, 2) << "\n";

	return 0;
}
