#include "AIMG.h"


TAimCreate			IAimCreate = NULL;
TAimFree			IAimFree = NULL;
TAimSetParams		IAimSetParams = NULL;
TAimProcess			IAimProcess = NULL;
TAimEnroll			IAimEnroll = NULL;
TAimDetect			IAimDetect = NULL;
TAimExtract			IAimExtract = NULL;
TAimMatch			IAimMatch = NULL;
TAimGetFeatureSize  IAimGetFeatureSize = NULL;
TAimManualEnroll	IAimManualEnroll = NULL;
TAimConvert			IAimConvert = NULL;
TGetCodeMeterDate	IGetCodeMeterDate = NULL;
TAimLandmark		IAimLandmark = NULL;

HMODULE LoadAPI(const char* fileName)
{
	HMODULE handle = LoadLibrary(fileName);

	if (!handle) {
		return NULL;
	}

	IAimCreate = (TAimCreate)GetProcAddress(handle, MAKEINTRESOURCE(1));
	IAimFree = (TAimFree)GetProcAddress(handle, MAKEINTRESOURCE(2));
	IAimSetParams = (TAimSetParams)GetProcAddress(handle, MAKEINTRESOURCE(3));
	IAimProcess = (TAimProcess)GetProcAddress(handle, MAKEINTRESOURCE(4));
	IAimEnroll = (TAimEnroll)GetProcAddress(handle, MAKEINTRESOURCE(5));
	IAimDetect = (TAimDetect)GetProcAddress(handle, MAKEINTRESOURCE(6));
	IAimExtract = (TAimExtract)GetProcAddress(handle, MAKEINTRESOURCE(7));
	IAimMatch = (TAimMatch)GetProcAddress(handle, MAKEINTRESOURCE(8));
	IAimGetFeatureSize = (TAimGetFeatureSize)GetProcAddress(handle, MAKEINTRESOURCE(9));
	IAimManualEnroll = (TAimManualEnroll)GetProcAddress(handle, MAKEINTRESOURCE(10));
	IAimConvert = (TAimConvert)GetProcAddress(handle, MAKEINTRESOURCE(11));
	IGetCodeMeterDate = (TGetCodeMeterDate)GetProcAddress(handle, MAKEINTRESOURCE(12));
	IAimLandmark = (TAimLandmark)GetProcAddress(handle, MAKEINTRESOURCE(13));

	return handle;
}

void FreeAPI(HMODULE handle)
{	
	if (handle) {
		FreeLibrary(handle);
	}
}
