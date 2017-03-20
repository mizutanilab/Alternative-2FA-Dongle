// Usbhidioc.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "Usbhidioc.h"

#include <wtypes.h>
#include <initguid.h>

extern "C" {
#include "hidsdi.h"
#include <setupapi.h>
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Application global variables
//	HIDP_CAPS		Capabilities;   08Mar2005 - Capabilities call fails on WinXP
	DWORD			cbBytesRead;
	PSP_DEVICE_INTERFACE_DETAIL_DATA	detailData;
	bool			DeviceDetected = false;
	HANDLE			DeviceHandle;
	DWORD			dwError;
	HANDLE			hEventObject;
	HANDLE			hDevInfo;
	GUID			HidGuid;
	OVERLAPPED		HIDOverlapped;
	char			InputReport[CUSBHIDIOC_BUFSIZE+1];
	ULONG			Length;
	DWORD			NumberOfBytesRead;
	HANDLE			ReadHandle;
	ULONG			Required;



/////////////////////////////////////////////////////////////////////////////
// CUsbhidioc

CUsbhidioc::CUsbhidioc()
{
}

CUsbhidioc::~CUsbhidioc()
{
}


BEGIN_MESSAGE_MAP(CUsbhidioc, CWnd)
	//{{AFX_MSG_MAP(CUsbhidioc)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUsbhidioc メッセージ ハンドラ

/////////////////////////////////////////////////////////////////////////////
// CUsbhidiocDlg message handlers


//DEFINE_GUID(GUID_CLASS_MOUSE, 0x378de44c, 0x56ef, 0x11d1, 0xbc, 0x8c, 0x00, 0xa0, 0xc9, 0x14, 0x05, 0xdd );
bool CUsbhidioc::FindTheHID()
{
	//Use a series of API calls to find a HID with a matching Vendor and Product ID.

	HIDD_ATTRIBUTES			   Attributes;
	SP_DEVICE_INTERFACE_DATA	   devInfoData;
	bool				   LastDevice = FALSE;
	int				   MemberIndex = 0;
	LONG				   Result;

	//These are the vendor and product IDs to look for.
//161127	const unsigned int VendorID = 0x04d8;    // Uses Microchip's Vendor ID.
//	const unsigned int ProductID = 0x0033;   // PICkit 2 Flash Starter Kit
	const unsigned int VendorID = 0x524D;    // 'RM'
	const unsigned int ProductID = 0x0011;   // productID: 0x11

	Length = 0;
	detailData = NULL;
	DeviceHandle=NULL;

	/*
	API function: HidD_GetHidGuid
	Get the GUID for all system HIDs.
	Returns: the GUID in HidGuid.
	*/

	HidD_GetHidGuid(&HidGuid);
	
	/*
	API function: SetupDiGetClassDevs
	Returns: a handle to a device information set for all installed devices.
	Requires: the GUID returned by GetHidGuid.
	*/
	
	hDevInfo=SetupDiGetClassDevs
		(&HidGuid,
		NULL, 
		NULL,
		DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);
		
	devInfoData.cbSize = sizeof(devInfoData);

	//Step through the available devices looking for the one we want. 
	//Quit on detecting the desired device or checking all available devices without success.
	MemberIndex = 0;
	LastDevice = FALSE;

	do
	{
		DeviceDetected=false;

		/*
		API function: SetupDiEnumDeviceInterfaces
		On return, MyDeviceInterfaceData contains the handle to a
		SP_DEVICE_INTERFACE_DATA structure for a detected device.
		Requires:
		The DeviceInfoSet returned in SetupDiGetClassDevs.
		The HidGuid returned in GetHidGuid.
		An index to specify a device.
		*/

		Result=SetupDiEnumDeviceInterfaces
			(hDevInfo,
			0,
			&HidGuid,
			MemberIndex,
			&devInfoData);

		if (Result != 0)
		{
			//A device has been detected, so get more information about it.

			/*
			API function: SetupDiGetDeviceInterfaceDetail
			Returns: an SP_DEVICE_INTERFACE_DETAIL_DATA structure
			containing information about a device.
			To retrieve the information, call this function twice.
			The first time returns the size of the structure in Length.
			The second time returns a pointer to the data in DeviceInfoSet.
			Requires:
			A DeviceInfoSet returned by SetupDiGetClassDevs
			The SP_DEVICE_INTERFACE_DATA structure returned by SetupDiEnumDeviceInterfaces.

			The final parameter is an optional pointer to an SP_DEV_INFO_DATA structure.
			This application doesn't retrieve or use the structure.
			If retrieving the structure, set
			MyDeviceInfoData.cbSize = length of MyDeviceInfoData.
			and pass the structure's address.
			*/

			//Get the Length value.
			//The call will return with a "buffer too small" error which can be ignored.
			Result = SetupDiGetDeviceInterfaceDetail
				(hDevInfo,
				&devInfoData,
				NULL,
				0,
				&Length,
				NULL);

			//Allocate memory for the hDevInfo structure, using the returned Length.
			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);

			//Set cbSize in the detailData structure.
			detailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        // **** IMPORTANT ****
        // sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) should equal 5.
        // In C++ Builder, go to Project/Options/Advanced Compiler/Data Alignment
        // and select "byte" align.

			//Call the function again, this time passing it the returned buffer size.
			Result = SetupDiGetDeviceInterfaceDetail
				(hDevInfo,
				&devInfoData,
				detailData,
				Length,
				&Required,
				NULL);

                        Result = GetLastError ();
			//Open a handle to the device.

			/*
			API function: CreateFile
			Returns: a handle that enables reading and writing to the device.
			Requires:
			The DevicePath in the detailData structure
			returned by SetupDiGetDeviceInterfaceDetail.
			*/

			DeviceHandle=CreateFile
				(detailData->DevicePath,
				GENERIC_READ|GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE,
				(LPSECURITY_ATTRIBUTES)NULL,
				OPEN_EXISTING,
				0,
				NULL);


			/*
			API function: HidD_GetAttributes
			Requests information from the device.
			Requires: the handle returned by CreateFile.
			Returns: a HIDD_ATTRIBUTES structure containing
			the Vendor ID, Product ID, and Product Version Number.
			Use this information to decide if the detected device is
			the one we're looking for.
			*/

			//Set the Size to the number of bytes in the structure.
			Attributes.Size = sizeof(Attributes);

			Result = HidD_GetAttributes(DeviceHandle, &Attributes);

			//Is it the desired device?
			DeviceDetected = false;

			if (Attributes.VendorID == VendorID) {
				if (Attributes.ProductID == ProductID) {
					//Both the Product and Vendor IDs match.
					DeviceDetected = true;
//                    GetDeviceCapabilities();  08Mar2005 - Capabilities call fails on WinXP
					PrepareForOverlappedTransfer();
				} //if (Attributes.ProductID == ProductID)
				else
					//The Product ID doesn't match.
					CloseHandle(DeviceHandle);
			} //if (Attributes.VendorID == VendorID)
			else
				//The Vendor ID doesn't match.
				CloseHandle(DeviceHandle);

		//Free the memory used by the detailData structure (no longer needed).
		free(detailData);
		}  //if (Result != 0)

		else
			//SetupDiEnumDeviceInterfaces returned 0, so there are no more devices to check.
			LastDevice=TRUE;

		//If we haven't found the device yet, and haven't tried every available device,
		//try the next one.
		MemberIndex = MemberIndex + 1;

	} //do
	while ((LastDevice == FALSE) && (DeviceDetected == false));

	//Free the memory reserved for hDevInfo by SetupDiClassDevs.
	SetupDiDestroyDeviceInfoList(hDevInfo);

	return DeviceDetected;
}

void CUsbhidioc::PrepareForOverlappedTransfer()
{
	//Get another handle to the device for the overlapped ReadFiles.
	ReadHandle=CreateFile
		(detailData->DevicePath,
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	//Get an event object for the overlapped structure.

	/*API function: CreateEvent
	Requires:
	  Security attributes or Null
	  Manual reset (true). Use ResetEvent to set the event object's state to non-signaled.
	  Initial state (true = signaled)
	  Event object name (optional)
	Returns: a handle to the event object
	*/

	if (hEventObject == 0)
	{
		hEventObject = CreateEvent
			(NULL,
			TRUE,
			TRUE,
			"");

	//Set the members of the overlapped structure.
	HIDOverlapped.hEvent = hEventObject;
	HIDOverlapped.Offset = 0;
	HIDOverlapped.OffsetHigh = 0;
	}
}


bool CUsbhidioc::ReadReport(unsigned char InBuffer[])
{
	DWORD	Result;

	//Read a report from the device.

	/*API call:ReadFile
	'Returns: the report in InputReport.
	'Requires: a device handle returned by CreateFile
	'(for overlapped I/O, CreateFile must be called with FILE_FLAG_OVERLAPPED),
	'the Input report length in bytes returned by HidP_GetCaps,
	'and an overlapped structure whose hEvent member is set to an event object.
	*/


        if (DeviceDetected == false) FindTheHID ();

        if (DeviceDetected) {
			Result = ReadFile
				(ReadHandle,
				InputReport,
				CUSBHIDIOC_BUFSIZE+1, //Capabilities.InputReportByteLength,     08Mar2005 - Capabilities call fails on WinXP
				&NumberOfBytesRead,
				(LPOVERLAPPED) &HIDOverlapped);


        		/*API call:WaitForSingleObject
				'Used with overlapped ReadFile.
				'Returns when ReadFile has received the requested amount of data or on timeout.
				'Requires an event object created with CreateEvent
				'and a timeout value in milliseconds.
				*/

			Result = WaitForSingleObject
			(hEventObject,
			1000);

			switch (Result) {
				case WAIT_OBJECT_0: {
					break;}
				case WAIT_TIMEOUT: {
					//Cancel the Read operation.
					/*API call: CancelIo
					Cancels the ReadFile
							Requires the device handle.
							Returns non-zero on success.
					*/

					Result = CancelIo(ReadHandle);

					//A timeout may mean that the device has been removed.
					//Close the device handles and set DeviceDetected = False
					//so the next access attempt will search for the device.
					CloseHandle(ReadHandle);
					CloseHandle(DeviceHandle);
					DeviceDetected = FALSE;
					break;}
				default: {
					break;}
			}

			// API call: ResetEvent
			// Sets the event object to non-signaled.
			// Requires a handle to the event object.
				// Returns non-zero on success.
			ResetEvent(hEventObject);

			// copy array to destination buffer, stripping off the report ID.
			unsigned int i;
			for (i=0; i < CUSBHIDIOC_BUFSIZE/*Capabilities.InputReportByteLength*/; ++i)
				InBuffer[i] = InputReport [i+1];
			Result = true;
        } else { // could not find the device
            Result = false;
		}

        return (bool)Result;
}

bool CUsbhidioc::WriteReport(unsigned char OutputBuffer[], unsigned int nBytes)
{
    DWORD   BytesWritten = 0;
    ULONG   Result;
    char    OutputReport [CUSBHIDIOC_BUFSIZE+1];
    unsigned int     i;

	//The first byte is the report number.
    OutputReport[0]=0;
    for (i=0; i < nBytes; i++) {OutputReport [i+1] = OutputBuffer[i];}
    for (i = nBytes+1; i < CUSBHIDIOC_BUFSIZE+1; ++i) {
        OutputReport [i] = 'Z';  // Pad buffer with Null Commands
	}
	/*
	API Function: WriteFile
	Sends a report to the device.
	Returns: success or failure.
	Requires:
	The device handle returned by CreateFile.
	The Output Report length returned by HidP_GetCaps,
	A report to send.
	*/

    if (DeviceDetected == false) FindTheHID ();

    if (DeviceDetected) {
        Result = WriteFile
		(DeviceHandle,
		OutputReport,
        CUSBHIDIOC_BUFSIZE+1, //Capabilities.OutputReportByteLength,  08Mar2005 - Capabilities call fails on WinXP
		&BytesWritten,
		NULL);

		//CString msg; msg.Format("%d", Result); AfxMessageBox(msg);
        if (Result == 0) {
            Result = GetLastError ();
			//The WriteFile failed, so close the handle, display a message,
			//and set DeviceDetected to FALSE so the next attempt will look for the device.
			CloseHandle(DeviceHandle);
			CloseHandle(ReadHandle);
			DeviceDetected = false;
        }
    } else {
        Result = false;
	}
    return (bool)Result;
}


// Close the channel
void CUsbhidioc::CloseReport ()
{
    if (DeviceDetected == true)
    {
        CancelIo(ReadHandle);

    //A timeout may mean that the device has been removed.
    //Close the device handles and set DeviceDetected = False
    //so the next access attempt will search for the device.
        CloseHandle(ReadHandle);
        CloseHandle(DeviceHandle);
        DeviceDetected = false;
    }
}

//08Mar2005 - Capabilities call fails on WinXP
//void CUsbhidioc::GetDeviceCapabilities()
//{
//    ULONG  Result;
	//Get the Capabilities structure for the device.
//	PHIDP_PREPARSED_DATA	PreparsedData;

	/*
	API function: HidD_GetPreparsedData
	Returns: a pointer to a buffer containing the information about the device's capabilities.
	Requires: A handle returned by CreateFile.
	There's no need to access the buffer directly,
	but HidP_GetCaps and other API functions require a pointer to the buffer.
	*/

//	Result = HidD_GetPreparsedData
//		(DeviceHandle,
//		&PreparsedData);

	/*
	API function: HidP_GetCaps
	Learn the device's capabilities.
	For standard devices such as joysticks, you can find out the specific
	capabilities of the device.
	For a custom device, the software will probably know what the device is capable of,
	and the call only verifies the information.
	Requires: the pointer to the buffer returned by HidD_GetPreparsedData.
	Returns: a Capabilities structure containing the information.
	*/

//	Result = HidP_GetCaps
//		(PreparsedData,
//		&Capabilities);

	//No need for PreparsedData any more, so free the memory it's using.
//	HidD_FreePreparsedData(PreparsedData);
//}

