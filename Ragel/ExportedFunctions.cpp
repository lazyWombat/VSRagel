#include <Windows.h>
#include "ExportedFunctions.h"
#include "rlscan.h"
#include "inputdata.h"
#include "common.h"

extern "C" __declspec(dllexport) wchar_t * __stdcall process(const wchar_t* inputFileName, const wchar_t * inputFileContent, int * hasErrors)
{
	hostLang = &hostLangCSharp;
	wstringstream out;
	wstringstream errors;

	pErrorStream = &errors;
	gblErrorCount = 0;

	InputData id;

	id.inputFileName = inputFileName;
	id.outputFileName = inputFileName;
	id.inStream = new wstringstream(inputFileContent);
	id.outStream = &out;

	InputItem *firstInputItem = new InputItem;
	firstInputItem->type = InputItem::HostData;
	firstInputItem->loc.fileName = id.inputFileName;
	firstInputItem->loc.line = 1;
	firstInputItem->loc.col = 1;
	id.inputItems.append(firstInputItem);

	Scanner scanner(id, id.inputFileName, *id.inStream, 0, 0, 0, false);
	scanner.do_scan();

	id.terminateAllParsers();
	id.prepareMachineGen();
	id.generateReduced();
	id.verifyWritesHaveData();
	id.writeOutput();

	wchar_t * result = NULL;

	if (gblErrorCount > 0)
	{
		*hasErrors = true;
		int length = (errors.str().length() + 1) * sizeof(wchar_t);
		result = (wchar_t*)::CoTaskMemAlloc(length);
		wcscpy_s(result, errors.str().length() + 1, errors.str().c_str());
	}
	else
	{
		*hasErrors = false;
		int length = (out.str().length() + 1) * sizeof(wchar_t);
		result = (wchar_t*)::CoTaskMemAlloc(length);
		wcscpy_s(result, out.str().length() + 1, out.str().c_str());
	}

	return result;
}