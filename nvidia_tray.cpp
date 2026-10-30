// nvidia_tray.cpp - Single-line GPU monitor for Windows console
//
// Copyright (C) 2026 zero-fret
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Build (MinGW): g++ -O2 nvidia_tray.cpp -o nvidia_tray.exe -luser32 -lshell32
// Build (MSVC):  cl /O2 nvidia_tray.cpp /link user32.lib shell32.lib
//
// Run: nvidia_tray.exe. Refreshes one line in place every 2 seconds.
//      Press Ctrl+C to exit.
// Output format: GPU name  power_draw/power_cap  mem_used/mem_total  util%

#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>

struct GpuInfo {
    std::string name;
    double power;
    double powerLimit;
    bool powerLimitNa;
    int memUsed;
    int memTotal;
    int utilization;
    bool valid;
};

// Run a shell command and return its stdout.
static std::string ExecCommand(const char* cmd) {
    std::string result;
    FILE* pipe = _popen(cmd, "r");
    if (!pipe) return result;
    char buf[512];
    while (fgets(buf, sizeof(buf), pipe) != NULL) {
        result += buf;
    }
    _pclose(pipe);
    return result;
}

static std::string Trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    return a == std::string::npos ? "" : s.substr(a, b - a + 1);
}

// Query the first GPU via nvidia-smi structured output.
// Uses enforced.power.limit (the actual power cap shown in the default
// nvidia-smi table) rather than power.limit, which is N/A on laptops.
static GpuInfo QueryGpu() {
    GpuInfo info = {};
    info.valid = false;
    std::string out = ExecCommand(
        "nvidia-smi --query-gpu=name,power.draw,enforced.power.limit,"
        "memory.used,memory.total,utilization.gpu --format=csv,noheader,nounits");
    if (out.empty()) return info;

    std::string line = out.substr(0, out.find('\n'));
    std::vector<std::string> parts;
    size_t pos = 0;
    while (true) {
        size_t comma = line.find(',', pos);
        if (comma == std::string::npos) {
            parts.push_back(line.substr(pos));
            break;
        }
        parts.push_back(line.substr(pos, comma - pos));
        pos = comma + 1;
    }
    if (parts.size() < 6) return info;

    info.name        = Trim(parts[0]);
    info.power       = atof(Trim(parts[1]).c_str());
    std::string pl   = Trim(parts[2]);
    info.powerLimitNa = (pl.find("N/A") != std::string::npos);
    info.powerLimit  = info.powerLimitNa ? 0.0 : atof(pl.c_str());
    info.memUsed     = atoi(Trim(parts[3]).c_str());
    info.memTotal    = atoi(Trim(parts[4]).c_str());
    info.utilization = atoi(Trim(parts[5]).c_str());
    info.valid       = true;
    return info;
}

int main() {
    const DWORD intervalMs = 2000;
    int lastLen = 0;
    while (true) {
        GpuInfo info = QueryGpu();
        char line[256];
        if (info.valid) {
            if (info.powerLimitNa) {
                snprintf(line, sizeof(line),
                         "%s  %.0f/N/A  %d/%dMiB  %d%%",
                         info.name.c_str(), info.power,
                         info.memUsed, info.memTotal, info.utilization);
            } else {
                snprintf(line, sizeof(line),
                         "%s  %.0f/%.0fW  %d/%dMiB  %d%%",
                         info.name.c_str(), info.power, info.powerLimit,
                         info.memUsed, info.memTotal, info.utilization);
            }
        } else {
            snprintf(line, sizeof(line), "nvidia-smi not found");
        }
        // \r back to line start; pad with spaces to erase leftover chars
        int curLen = (int)strlen(line);
        printf("\r%s", line);
        if (curLen < lastLen) {
            printf("%*s", lastLen - curLen, "");
        }
        fflush(stdout);
        lastLen = curLen;
        Sleep(intervalMs);
    }
    return 0;
}
