#pragma once
#include <stdexcept>
using namespace std;

namespace RagelFuncs
{
	extern "C" __declspec(dllexport) wchar_t *process(const wchar_t* inputFileName, const wchar_t * inputFileContent, int * hasErrors);
}
