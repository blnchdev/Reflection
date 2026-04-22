#pragma once
#include "Definitions/Globals.h"
#include "Components/Memory Manager/MemoryStructs.h"

namespace Templates
{
	using namespace Memory;

	inline Field StructPointer( const uintptr_t Offset, const std::string& Label, const Info* Structure )
	{
		auto F = Field( T_pClass, Offset, Label );

		if ( Structure )
		{
			F.Embedded = Structure->Label;
		}

		return F;
	}

	inline Field EmbeddedStruct( const uintptr_t Offset, const std::string& Label, const Info* Structure )
	{
		auto F = Field( T_EmbeddedClass, Offset, Label );

		if ( Structure )
		{
			F.Embedded = Structure->Label;
		}

		return F;
	}

	inline Field pCharString( const uintptr_t Offset, const std::string& Label )
	{
		auto F = Field( T_char_array, Offset, Label );
		F.Size = 64;
		return F;
	}

	inline Field pWideCharString( const uintptr_t Offset, const std::string& Label )
	{
		auto F = Field( T_wide_array, Offset, Label );
		F.Size = 128;
		return F;
	}

	inline Info LIST_ENTRY = Info{
		_( "LIST_ENTRY" ),
		{
			Field( T_rawPointer, 0x0, _( "Flink" ) ),
			Field( T_rawPointer, 0x8, _( "Blink" ) ),
		}
	};

	inline auto PEB_LDR_DATA = Info{
		_( "PEB_LDR_DATA" ),
		{
			Field( T_uint32, 0x0, _( "Length" ) ),
			Field( T_bool, 0x4, _( "Initialized" ) ),
			Field( T_qword, 0x8, _( "SsHandle" ) ),
			EmbeddedStruct( 0x10, _( "InLoadOrderModuleList" ), &LIST_ENTRY ),
			EmbeddedStruct( 0x20, _( "InMemoryOrderModuleList" ), &LIST_ENTRY ),
			EmbeddedStruct( 0x30, _( "InInitializationOrderModuleList" ), &LIST_ENTRY ),
			Field( T_rawPointer, 0x40, _( "EntryInProgress" ) ),
			Field( T_bool, 0x48, _( "ShutdownInProgress" ) ),
			Field( T_qword, 0x50, _( "ShutdownThreadId" ) )
		}
	};

	inline auto UNICODE_STRING = Info{
		_( "UNICODE_STRING" ),
		{
			Field( T_uint16, 0x0, _( "Length" ) ),
			Field( T_uint16, 0x2, _( "MaximumLength" ) ),
			pWideCharString( 0x8, _( "Buffer" ) )
		}
	};

	inline auto CURDIR = Info{
		_( "CURDIR" ),
		{
			EmbeddedStruct( 0x0, _( "DosPath" ), &UNICODE_STRING ),
			Field( T_qword, 0x10, _( "Handle" ) ),
		}
	};

	inline auto RTL_DRIVE_LETTER_CURDIR = Info{
		_( "RTL_DRIVE_LETTER_CURDIR" ),
		{
			Field( T_uint16, 0x0, _( "Flags" ) ),
			Field( T_uint16, 0x2, _( "Length" ) ),
			Field( T_uint32, 0x4, _( "TimeStamp" ) ),
			EmbeddedStruct( 0x8, _( "DosPath" ), &UNICODE_STRING ),
		}
	};

	inline auto RTL_USER_PROCESS_PARAMETERS = Info{
		_( "RTL_USER_PROCESS_PARAMETERS" ),
		{
			Field( T_uint32, 0x000, _( "MaximumLength" ) ),
			Field( T_uint32, 0x004, _( "Length" ) ),
			Field( T_dword, 0x008, _( "Flags" ) ),
			Field( T_dword, 0x00C, _( "DebugFlags" ) ),
			Field( T_qword, 0x010, _( "ConsoleHandle" ) ),
			Field( T_dword, 0x018, _( "ConsoleFlags" ) ),
			Field( T_qword, 0x020, _( "StandardInput" ) ),
			Field( T_qword, 0x028, _( "StandardOutput" ) ),
			Field( T_qword, 0x030, _( "StandardError" ) ),
			EmbeddedStruct( 0x038, _( "CurrentDirectory" ), &CURDIR ),
			EmbeddedStruct( 0x050, _( "DllPath" ), &UNICODE_STRING ),
			EmbeddedStruct( 0x060, _( "ImagePathName" ), &UNICODE_STRING ),
			EmbeddedStruct( 0x070, _( "CommandLine" ), &UNICODE_STRING ),
			Field( T_rawPointer, 0x080, _( "Environment" ) ),
			Field( T_uint32, 0x088, _( "StartingX" ) ),
			Field( T_uint32, 0x08C, _( "StartingY" ) ),
			Field( T_uint32, 0x090, _( "CountX" ) ),
			Field( T_uint32, 0x094, _( "CountY" ) ),
			Field( T_uint32, 0x098, _( "CountCharsX" ) ),
			Field( T_uint32, 0x09C, _( "CountCharsY" ) ),
			Field( T_dword, 0x0A0, _( "FillAttribute" ) ),
			Field( T_dword, 0x0A4, _( "WindowFlags" ) ),
			Field( T_dword, 0x0A8, _( "ShowWindowFlags" ) ),
			EmbeddedStruct( 0x0B0, _( "WindowTitle" ), &UNICODE_STRING ),
			EmbeddedStruct( 0x0C0, _( "DesktopInfo" ), &UNICODE_STRING ),
			EmbeddedStruct( 0x0D0, _( "ShellInfo" ), &UNICODE_STRING ),
			EmbeddedStruct( 0x0E0, _( "RuntimeData" ), &UNICODE_STRING ),
			EmbeddedStruct( 0x0F0, _( "CurrentDirectories" ), &RTL_DRIVE_LETTER_CURDIR ), // [32]
			Field( T_uint64, 0x3F0, _( "EnvironmentSize" ) ),
			Field( T_uint64, 0x3F8, _( "EnvironmentVersion" ) ),
			Field( T_rawPointer, 0x400, _( "PackageDependencyData" ) ),
			Field( T_dword, 0x408, _( "ProcessGroupId" ) ),
			Field( T_dword, 0x40C, _( "LoaderThreads" ) ),
			EmbeddedStruct( 0x410, _( "RedirectionDllName" ), &UNICODE_STRING ),
			EmbeddedStruct( 0x420, _( "HeapPartitionName" ), &UNICODE_STRING ),
			Field( T_rawPointer, 0x430, _( "DefaultThreadpoolCpuSetMasks" ) ),
			Field( T_uint32, 0x438, _( "DefaultThreadpoolCpuSetMaskCount" ) ),
			Field( T_uint32, 0x43C, _( "DefaultThreadpoolThreadMaximum" ) ),
			Field( T_dword, 0x440, _( "HeapMemoryTypeMask" ) ),
		}
	};

	inline auto KERNEL_CALLBACK_TABLE = Info{
		_( "KERNEL_CALLBACK_TABLE" ),
		{
			Field( T_rawPointer, 0x0, _( "__fnCOPYDATA" ) ),
			Field( T_rawPointer, 0x8, _( "__fnCOPYGLOBALDATA" ) ),
			Field( T_rawPointer, 0x10, _( "__fnEMPTY1" ) ),
			Field( T_rawPointer, 0x18, _( "__fnNCDESTROY" ) ),
			Field( T_rawPointer, 0x20, _( "__fnDWORDOPTINLPMSG" ) ),
			Field( T_rawPointer, 0x28, _( "__fnINOUTDRAG" ) ),
			Field( T_rawPointer, 0x30, _( "__fnGETTEXTLENGTHS1" ) ),
			Field( T_rawPointer, 0x38, _( "__fnINCNTOUTSTRING" ) ),
			Field( T_rawPointer, 0x40, _( "__fnINCNTOUTSTRINGNULL" ) ),
			Field( T_rawPointer, 0x48, _( "__fnINLPCOMPAREITEMSTRUCT" ) ),
			Field( T_rawPointer, 0x50, _( "__fnINLPCREATESTRUCT" ) ),
			Field( T_rawPointer, 0x58, _( "__fnINLPDELETEITEMSTRUCT" ) ),
			Field( T_rawPointer, 0x60, _( "__fnINLPDRAWITEMSTRUCT" ) ),
			Field( T_rawPointer, 0x68, _( "__fnPOPTINLPUINT1" ) ),
			Field( T_rawPointer, 0x70, _( "__fnPOPTINLPUINT2" ) ),
			Field( T_rawPointer, 0x78, _( "__fnINLPMDICREATESTRUCT" ) ),
			Field( T_rawPointer, 0x80, _( "__fnINOUTLPMEASUREITEMSTRUCT" ) ),
			Field( T_rawPointer, 0x88, _( "__fnINLPWINDOWPOS" ) ),
			Field( T_rawPointer, 0x90, _( "__fnINOUTLPPOINT51" ) ),
			Field( T_rawPointer, 0x98, _( "__fnINOUTLPSCROLLINFO" ) ),
			Field( T_rawPointer, 0xA0, _( "__fnINOUTLPRECT" ) ),
			Field( T_rawPointer, 0xA8, _( "__fnINOUTNCCALCSIZE" ) ),
			Field( T_rawPointer, 0xB0, _( "__fnINOUTLPPOINT52" ) ),
			Field( T_rawPointer, 0xB8, _( "__fnINPAINTCLIPBRD" ) ),
			Field( T_rawPointer, 0xC0, _( "__fnINSIZECLIPBRD" ) ),
			Field( T_rawPointer, 0xC8, _( "__fnINDESTROYCLIPBRD" ) ),
			Field( T_rawPointer, 0xD0, _( "__fnINSTRINGNULL1" ) ),
			Field( T_rawPointer, 0xD8, _( "__fnINSTRINGNULL2" ) ),
			Field( T_rawPointer, 0xE0, _( "__fnINDEVICECHANGE" ) ),
			Field( T_rawPointer, 0xE8, _( "__fnPOWERBROADCAST" ) ),
			Field( T_rawPointer, 0xF0, _( "__fnINLPUAHDRAWMENU1" ) ),
			Field( T_rawPointer, 0xF8, _( "__fnOPTOUTLPDWORDOPTOUTLPDWORD1" ) ),
			Field( T_rawPointer, 0x100, _( "__fnOPTOUTLPDWORDOPTOUTLPDWORD2" ) ),
			Field( T_rawPointer, 0x108, _( "__fnOUTDWORDINDWORD" ) ),
			Field( T_rawPointer, 0x110, _( "__fnOUTLPRECT" ) ),
			Field( T_rawPointer, 0x118, _( "__fnOUTSTRING" ) ),
			Field( T_rawPointer, 0x120, _( "__fnPOPTINLPUINT3" ) ),
			Field( T_rawPointer, 0x128, _( "__fnPOUTLPINT" ) ),
			Field( T_rawPointer, 0x130, _( "__fnSENTDDEMSG" ) ),
			Field( T_rawPointer, 0x138, _( "__fnINOUTSTYLECHANGE1" ) ),
			Field( T_rawPointer, 0x140, _( "__fnHkINDWORD" ) ),
			Field( T_rawPointer, 0x148, _( "__fnHkINLPCBTACTIVATESTRUCT" ) ),
			Field( T_rawPointer, 0x150, _( "__fnHkINLPCBTCREATESTRUCT" ) ),
			Field( T_rawPointer, 0x158, _( "__fnHkINLPDEBUGHOOKSTRUCT" ) ),
			Field( T_rawPointer, 0x160, _( "__fnHkINLPMOUSEHOOKSTRUCTEX1" ) ),
			Field( T_rawPointer, 0x168, _( "__fnHkINLPKBDLLHOOKSTRUCT" ) ),
			Field( T_rawPointer, 0x170, _( "__fnHkINLPMSLLHOOKSTRUCT" ) ),
			Field( T_rawPointer, 0x178, _( "__fnHkINLPMSG" ) ),
			Field( T_rawPointer, 0x180, _( "__fnHkINLPRECT" ) ),
			Field( T_rawPointer, 0x188, _( "__fnHkOPTINLPEVENTMSG" ) ),
			Field( T_rawPointer, 0x190, _( "__xxxClientCallDelegateThread" ) ),
			Field( T_rawPointer, 0x198, _( "__ClientCallDummyCallback1" ) ),
			Field( T_rawPointer, 0x1A0, _( "__ClientCallDummyCallback2" ) ),
			Field( T_rawPointer, 0x1A8, _( "__fnSHELLWINDOWMANAGEMENTCALLOUT" ) ),
			Field( T_rawPointer, 0x1B0, _( "__fnSHELLWINDOWMANAGEMENTNOTIFY" ) ),
			Field( T_rawPointer, 0x1B8, _( "__ClientCallDummyCallback3" ) ),
			Field( T_rawPointer, 0x1C0, _( "__xxxClientCallDitThread" ) ),
			Field( T_rawPointer, 0x1C8, _( "__xxxClientEnableMMCSS" ) ),
			Field( T_rawPointer, 0x1D0, _( "__xxxClientUpdateDpi" ) ),
			Field( T_rawPointer, 0x1D8, _( "__xxxClientExpandStringW" ) ),
			Field( T_rawPointer, 0x1E0, _( "__ClientCopyDDEIn1" ) ),
			Field( T_rawPointer, 0x1E8, _( "__ClientCopyDDEIn2" ) ),
			Field( T_rawPointer, 0x1F0, _( "__ClientCopyDDEOut1" ) ),
			Field( T_rawPointer, 0x1F8, _( "__ClientCopyDDEOut2" ) ),
			Field( T_rawPointer, 0x200, _( "__ClientCopyImage" ) ),
			Field( T_rawPointer, 0x208, _( "__ClientEventCallback" ) ),
			Field( T_rawPointer, 0x210, _( "__ClientFindMnemChar" ) ),
			Field( T_rawPointer, 0x218, _( "__ClientFreeDDEHandle" ) ),
			Field( T_rawPointer, 0x220, _( "__ClientFreeLibrary" ) ),
			Field( T_rawPointer, 0x228, _( "__ClientGetCharsetInfo" ) ),
			Field( T_rawPointer, 0x230, _( "__ClientGetDDEFlags" ) ),
			Field( T_rawPointer, 0x238, _( "__ClientGetDDEHookData" ) ),
			Field( T_rawPointer, 0x240, _( "__ClientGetListboxString" ) ),
			Field( T_rawPointer, 0x248, _( "__ClientGetMessageMPH" ) ),
			Field( T_rawPointer, 0x250, _( "__ClientLoadImage" ) ),
			Field( T_rawPointer, 0x258, _( "__ClientLoadLibrary" ) ),
			Field( T_rawPointer, 0x260, _( "__ClientLoadMenu" ) ),
			Field( T_rawPointer, 0x268, _( "__ClientLoadLocalT1Fonts" ) ),
			Field( T_rawPointer, 0x270, _( "__ClientPSMTextOut" ) ),
			Field( T_rawPointer, 0x278, _( "__ClientLpkDrawTextEx" ) ),
			Field( T_rawPointer, 0x280, _( "__ClientExtTextOutW" ) ),
			Field( T_rawPointer, 0x288, _( "__ClientGetTextExtentPointW" ) ),
			Field( T_rawPointer, 0x290, _( "__ClientCharToWchar" ) ),
			Field( T_rawPointer, 0x298, _( "__ClientAddFontResourceW" ) ),
			Field( T_rawPointer, 0x2A0, _( "__ClientThreadSetup" ) ),
			Field( T_rawPointer, 0x2A8, _( "__ClientDeliverUserApc" ) ),
			Field( T_rawPointer, 0x2B0, _( "__ClientNoMemoryPopup" ) ),
			Field( T_rawPointer, 0x2B8, _( "__ClientMonitorEnumProc" ) ),
			Field( T_rawPointer, 0x2C0, _( "__ClientCallWinEventProc" ) ),
			Field( T_rawPointer, 0x2C8, _( "__ClientWaitMessageExMPH" ) ),
			Field( T_rawPointer, 0x2D0, _( "__ClientCallDummyCallback4" ) ),
			Field( T_rawPointer, 0x2D8, _( "__ClientCallDummyCallback5" ) ),
			Field( T_rawPointer, 0x2E0, _( "__ClientImmLoadLayout" ) ),
			Field( T_rawPointer, 0x2E8, _( "__ClientImmProcessKey" ) ),
			Field( T_rawPointer, 0x2F0, _( "__fnIMECONTROL" ) ),
			Field( T_rawPointer, 0x2F8, _( "__fnINWPARAMDBCSCHAR" ) ),
			Field( T_rawPointer, 0x300, _( "__fnGETTEXTLENGTHS2" ) ),
			Field( T_rawPointer, 0x308, _( "__ClientCallDummyCallback6" ) ),
			Field( T_rawPointer, 0x310, _( "__ClientLoadStringW" ) ),
			Field( T_rawPointer, 0x318, _( "__ClientLoadOLE" ) ),
			Field( T_rawPointer, 0x320, _( "__ClientRegisterDragDrop" ) ),
			Field( T_rawPointer, 0x328, _( "__ClientRevokeDragDrop" ) ),
			Field( T_rawPointer, 0x330, _( "__fnINOUTMENUGETOBJECT" ) ),
			Field( T_rawPointer, 0x338, _( "__ClientPrinterThunk" ) ),
			Field( T_rawPointer, 0x340, _( "__fnOUTLPCOMBOBOXINFO" ) ),
			Field( T_rawPointer, 0x348, _( "__fnOUTLPSCROLLBARINFO" ) ),
			Field( T_rawPointer, 0x350, _( "__fnINLPUAHDRAWMENU2" ) ),
			Field( T_rawPointer, 0x358, _( "__fnINLPUAHDRAWMENUITEM" ) ),
			Field( T_rawPointer, 0x360, _( "__fnINLPUAHDRAWMENU3" ) ),
			Field( T_rawPointer, 0x368, _( "__fnINOUTLPUAHMEASUREMENUITEM" ) ),
			Field( T_rawPointer, 0x370, _( "__fnINLPUAHDRAWMENU4" ) ),
			Field( T_rawPointer, 0x378, _( "__fnOUTLPTITLEBARINFOEX" ) ),
			Field( T_rawPointer, 0x380, _( "__fnTOUCH" ) ),
			Field( T_rawPointer, 0x388, _( "__fnGESTURE" ) ),
			Field( T_rawPointer, 0x390, _( "__fnPOPTINLPUINT4" ) ),
			Field( T_rawPointer, 0x398, _( "__fnPOPTINLPUINT5" ) ),
			Field( T_rawPointer, 0x3A0, _( "__xxxClientCallDefaultInputHandler" ) ),
			Field( T_rawPointer, 0x3A8, _( "__fnEMPTY2" ) ),
			Field( T_rawPointer, 0x3B0, _( "__ClientRimDevCallback" ) ),
			Field( T_rawPointer, 0x3B8, _( "__xxxClientCallMinTouchHitTestingCallback" ) ),
			Field( T_rawPointer, 0x3C0, _( "__ClientCallLocalMouseHooks" ) ),
			Field( T_rawPointer, 0x3C8, _( "__xxxClientBroadcastThemeChange" ) ),
			Field( T_rawPointer, 0x3D0, _( "__xxxClientCallDevCallbackSimple" ) ),
			Field( T_rawPointer, 0x3D8, _( "__xxxClientAllocWindowClassExtraBytes" ) ),
			Field( T_rawPointer, 0x3E0, _( "__xxxClientFreeWindowClassExtraBytes" ) ),
			Field( T_rawPointer, 0x3E8, _( "__fnGETWINDOWDATA" ) ),
			Field( T_rawPointer, 0x3F0, _( "__fnINOUTSTYLECHANGE2" ) ),
			Field( T_rawPointer, 0x3F8, _( "__fnHkINLPMOUSEHOOKSTRUCTEX2" ) ),
			Field( T_rawPointer, 0x400, _( "__xxxClientCallDefWindowProc" ) ),
			Field( T_rawPointer, 0x408, _( "__fnSHELLSYNCDISPLAYCHANGED" ) ),
			Field( T_rawPointer, 0x410, _( "__fnHkINLPCHARHOOKSTRUCT" ) ),
			Field( T_rawPointer, 0x418, _( "__fnINTERCEPTEDWINDOWACTION" ) ),
			Field( T_rawPointer, 0x420, _( "__xxxTooltipCallback" ) ),
			Field( T_rawPointer, 0x428, _( "__xxxClientInitPSBInfo" ) ),
			Field( T_rawPointer, 0x430, _( "__xxxClientDoScrollMenu" ) ),
			Field( T_rawPointer, 0x438, _( "__xxxClientEndScroll" ) ),
			Field( T_rawPointer, 0x440, _( "__xxxClientDrawSize" ) ),
			Field( T_rawPointer, 0x448, _( "__xxxClientDrawScrollBar" ) ),
			Field( T_rawPointer, 0x450, _( "__xxxClientHitTestScrollBar" ) ),
			Field( T_rawPointer, 0x458, _( "__xxxClientTrackInit" ) )
		}
	};

	inline auto PEB = Info{
		_( "PEB" ),
		{
			Field( T_bool, 0x0, _( "InheritedAddressSpace" ) ),
			Field( T_bool, 0x1, _( "ReadImageFileExecOptions" ) ),
			Field( T_bool, 0x2, _( "BeingDebugged" ) ),
			Field( T_byte, 0x3, _( "Bitfield" ) ),
			Field( T_qword, 0x8, _( "Mutant" ) ),
			Field( T_rawPointer, 0x10, _( "ImageBaseAddress" ) ),
			StructPointer( 0x18, _( "Ldr" ), &PEB_LDR_DATA ),
			StructPointer( 0x20, _( "ProcessParameters" ), &RTL_USER_PROCESS_PARAMETERS ),
			Field( T_rawPointer, 0x28, _( "SubSystemData" ) ),
			Field( T_rawPointer, 0x30, _( "ProcessHeap" ) ),
			Field( T_rawPointer, 0x38, _( "FastPebLock" ) ),
			Field( T_rawPointer, 0x40, _( "AtlThunkSListPtr" ) ),
			Field( T_rawPointer, 0x48, _( "IFEOKey" ) ),
			Field( T_uint32, 0x50, _( "CrossProcessFlags" ) ),
			StructPointer( 0x58, _( "KernelCallbackTable" ), &KERNEL_CALLBACK_TABLE ),
			Field( T_uint32, 0x60, _( "SystemReserved" ) ),
		}
	};

	inline auto Empty = Info{
		_( "Class" ),
		{

		}
	};
}
