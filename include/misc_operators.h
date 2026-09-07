#pragma once
#define NOMINMAX
#undef APIENTRY
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <string>
#include <cassert>
#include "data_type.h"


namespace DataInput
{
    template<typename T>
    void CheckAndSetData(
        T& Input,
        const std::string& InputCommand,
        const std::string& FailureCommand)
    {
        bool InputSet = false;

        while (!InputSet)
        {
            std::cout << InputCommand;

            std::string line;

            std::getline(std::cin, line);

            std::stringstream ss(line);

            if (ss >> Input)
            {
                InputSet = true;
            }
            else
            {
                std::cout << FailureCommand << "\n";
            }
        }
    }
}

template<typename T, typename S>
    requires std::is_arithmetic_v<T> && requires(S s, T value)
{
    { s(value) } -> std::same_as<bool>;
}
void GetInput(T& OutValue, S ConditionFunction, const std::string& InFailureString)
{
    while (true)
    {
        std::string input;
        std::getline(std::cin, input);
        std::stringstream ss(input);
        T value;
        if (ss >> value)
        {
            ss >> std::ws;

            if (ss.eof())
            {
                if (ConditionFunction(value))
                {
                    OutValue = value;
                    break;
                }
            }

			std::cout << InFailureString << "\n";
            continue;
        }
        else
        {
            if constexpr (std::is_same_v<T, int>)
            {
                std::cout << "Invalid input. Please enter a valid integer: ";
                continue;
            }

            if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>)
            {
                std::cout << "Invalid input. Please enter a valid floating-point number: ";
                continue;
            }

            if constexpr (std::is_same_v<T, bool>)
            {
                std::cout << "Invalid input. Please enter 1 for true or 0 for false: ";
                continue;
			}

            std::cout << "Invalid input. Please enter a valid number: ";
        }
	}
}

namespace SimulationTime
{
    struct SimTime
    {
        SimTime()
        {
            CurrentTime = std::chrono::high_resolution_clock::now();
            PreviousTime = CurrentTime;
            FrameDelta = CurrentTime - PreviousTime;
        }

        inline  void Update()
        {
            PreviousTime = CurrentTime;
            CurrentTime = std::chrono::high_resolution_clock::now();
            FrameDelta = CurrentTime - PreviousTime;
        }

        std::chrono::steady_clock::duration GetFrameDelta() const
        {
            return FrameDelta;
        }

        std::chrono::steady_clock::time_point GetCurrentTime() const
        {
            return CurrentTime;
        }

        std::chrono::steady_clock::time_point GetPreviousTime() const
        {
            return PreviousTime;
        }

        static std::string GetTimeString()
        {
            auto now = std::chrono::system_clock::now();

            std::time_t timeNow = std::chrono::system_clock::to_time_t(now);

            std::tm localTime{};

#ifdef _WIN32
            localtime_s(&localTime, &timeNow);
#else
            localtime_s(&timeNow, &localTime);
#endif // _WIN32

            std::stringstream stream;

            stream << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S");

            return stream.str();
        }

    private :

        std::chrono::steady_clock::time_point CurrentTime;
        std::chrono::steady_clock::time_point PreviousTime;
        std::chrono::steady_clock::duration FrameDelta;
    };
}

namespace sim_fileOps
{
    namespace fs = std::filesystem;

    inline fs::path GetExecutableDirectory()
    {
        wchar_t buffer[MAX_PATH];

        GetModuleFileNameW(nullptr, buffer, MAX_PATH);

        return fs::path(buffer).parent_path();
    }

    inline fs::path CreateTimestampedOutputFolder(const fs::path& InParentPath, const std::string& InFolderName)
    {
        std::string timestamp = SimulationTime::SimTime::GetTimeString();

        fs::path folderPath = InParentPath / (InFolderName + "_" + timestamp);

        fs::create_directories(folderPath);

        return folderPath;
    }

    inline fs::path CreateOutputCSVFile(const std::string& InFilename, const fs::path& InFolderPath, bool bCreateSubFolder)
    {
        fs::create_directories(InFolderPath);

        fs::path folderPath = InFolderPath;

        if (bCreateSubFolder)
        {
            std::string timeStamp = SimulationTime::SimTime::GetTimeString();

            std::string subFolderName = InFilename + "_" + timeStamp;

            folderPath = InFolderPath / subFolderName;

            fs::create_directories(folderPath);
        }

        fs::path filePath = folderPath / (InFilename + ".csv");

        std::ofstream file(filePath);

        if (!file.is_open())
        {
            return {};
        }

        return filePath;
    }

    inline bool AppendToFile(fs::path InFilePath, const std::string& InString)
    {
        if (!fs::exists(InFilePath)) return false;

        std::ofstream file(InFilePath, std::ios::app);

        file << InString;

        return true;
    }

    inline std::string ReadFile(const std::string& FilePath)
    {
        std::ifstream file(FilePath);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << FilePath << std::endl;
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}


// ==========================================
// Mouse State
// ==========================================
struct MouseState
{
    Double2 WindowPosition = Double2(0.0, 0.0);
    Double2 PreviousWindowPosition = Double2(0.0, 0.0);
    Double2 DeltaWindow = Double2(0.0, 0.0);

    Double2 GridPosition = Double2(-1.0, -1.0);
    Double2 PreviousGridPosition = Double2(-1.0, -1.0);
    Double2 DeltaGrid = Double2(0.0, 0.0);

    Double2 MouseVelocity = Double2(0.0, 0.0);

    Int2 MouseSteps = Int2(0, 0);

    bool bLeftButtonDown = false;
    bool bRightButtonDown = false;

    bool IsMouseInsideGrid() const
    {
        return GridPosition.x >= 0.0 && GridPosition.y >= 0.0 && PreviousGridPosition.x >= 0.0 && PreviousGridPosition.y >= 0.0;
    }

    Int2 GetGridPositionAsInt2() const
    {
        return Int2(static_cast<int>(GridPosition.x), static_cast<int>(GridPosition.y));
    }

    Int2 GetPreviousGridPositionAsInt2() const
    {
        return Int2(static_cast<int>(PreviousGridPosition.x), static_cast<int>(PreviousGridPosition.y));
    }
};



struct FrameProfiler
{
    uint64_t FrameCount = 0;

    double CurrentTime = 0.0;
    double PreviousTime = 0.0;
    double DeltaTime = 0.0;

    double FPS = 0.0;
};