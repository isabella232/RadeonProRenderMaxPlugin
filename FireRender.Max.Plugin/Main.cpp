/**********************************************************************
Copyright 2020 Advanced Micro Devices, Inc
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 * 
 * DllMain and some basic functions loaded directly by 3ds Max plugin mechanism to initialize the plugin after the DLL is loaded
 *********************************************************************************************************************************/ 

#include "Common.h"
#include "ClassDescs.h"
#include "utils/Utils.h"
#include <iparamb2.h>
#include <direct.h>

#include "FireRenderDiffuseMtl.h"
#include "FireRenderBlendMtl.h"
#include "FireRenderAddMtl.h"
#include "FireRenderMicrofacetMtl.h"
#include "FireRenderReflectionMtl.h"
#include "FireRenderAmbientOcclusionMtl.h"
#include "FireRenderArithmMtl.h"
#include "FireRenderInputLUMtl.h"
#include "FireRenderBlendValueMtl.h"
#include "FireRenderRefractionMtl.h"
#include "FireRenderMFRefractionMtl.h"
#include "FireRenderTransparentMtl.h"
#include "FireRenderWardMtl.h"
#include "FireRenderEmissiveMtl.h"
#include "FireRenderFresnelMtl.h"
#include "FireRenderStandardMtl.h"
#include "FireRenderColorMtl.h"
#include "FireRenderAvgMtl.h"
#include "FireRenderOrenNayarMtl.h"
#include "FireRenderFresnelSchlickMtl.h"
#include "FireRenderDisplacementMtl.h"
#include "FireRenderNormalMtl.h"
#include "FireRenderDiffuseRefractionMtl.h"
#include "FireRenderMaterialMtl.h"
#include "FireRenderUberMtl.h"
#include "FireRenderUberMtlv2.h"
#include "FireRenderUberMtlv3.h"
#include "FireRenderVolumeMtl.h"
#include "FireRenderPbrMtl.h"
#include "FireRenderShadowCatcherMtl.h"
#include "FireRenderColourCorrectionMtl.h"

#include "XMLMaterialExporter.h"
#include "FireRenderEnvironment.h"
#include "FireRenderAnalyticalSun.h"
#include "FireRenderPortalLight.h"
#include "FireRenderIESLight.h"
#include "physical/FireRenderPhysicalLight.h"

#include "RprExporter.h"

#include "BgManager.h"
#include "TmManager.h"
#include "CamManager.h"
#include "MPManager.h"
#include "ScopeManager.h"
#include "PRManager.h"
#include "utils\Utils.h"

#pragma comment (lib, "Release/maxutil.lib")
#pragma comment (lib, "Release/core.lib")
#pragma comment (lib, "Release/paramblk2.lib")
#pragma comment (lib, "Release/bmm.lib")
#pragma comment (lib, "Release/geom.lib")
#pragma comment (lib, "Release/mesh.lib")
#pragma comment (lib, "Release/poly.lib")
#pragma comment (lib, "Release/gfx.lib")
#pragma comment (lib, "Release/GraphicsDriver.lib")
#pragma comment (lib, "Release/DefaultRenderItems.lib")
#pragma comment (lib, "Release/maxscrpt.lib")
#pragma comment (lib, "Release/gup.lib")
#pragma comment (lib, "Release/ManipSys.lib")

extern "C" void DisableGltfExport();

HINSTANCE FireRender::fireRenderHInstance;

// This is the required signature for some of the functions to be loaded by 3ds Max when the DLL is loaded
#define EXPORT_TO_MAX extern "C" __declspec(dllexport)

namespace
{
	std::vector<ClassDesc*> gClassInstances;
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID /*lpvReserved*/)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
	{
        // Save the HINSTANCE of this plugin
        FireRender::fireRenderHInstance = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
	}

    return true;
}

/// Returns a description that is displayed in 3ds Max plugin manager
EXPORT_TO_MAX const TCHAR* LibDescription()
{
    return _T("Radeon ProRender for 3ds Max(R)");
}

/// Tells 3ds Max how many plugins are implemented in this DLL. Determines the indices with which LibClassDesc is called later
EXPORT_TO_MAX int LibNumberClasses()
{
	return int_cast(gClassInstances.size());
}

/// Returns the class descriptors for all plugins implemented in this DLL
EXPORT_TO_MAX ClassDesc* LibClassDesc(int i)
{
	if (i < gClassInstances.size())
		return gClassInstances[i];

	FASSERT(false);
    return NULL;
}

/// Has to always return Get3DSMAXVersion()
EXPORT_TO_MAX ULONG LibVersion()
{
    return Get3DSMAXVersion();
}

int GetAOVElementClassDescCount();
ClassDesc2& GetAOVElementClassDesc(int);

/// Called by 3ds Max immediately after 3ds Max loads the plugin. Any initialization should be done here (and not in DllMain)
EXPORT_TO_MAX int LibInitialize()
{
	std::wstring pluginPath = FireRender::GetModuleFolder();

	if ( pluginPath.empty() )
	{
		MessageBox(0, L"Failed to get module name. Plugin will not be loaded.", L"Radeon ProRender", MB_OK | MB_ICONEXCLAMATION);

		return 0;
	}

	// getting the root folder of plugin
	std::wstring::size_type pos = pluginPath.find(L"plug-ins");

	if (std::wstring::npos == pos)
	{
		MessageBox(0, L"Failed to get module path. Plugin will not be loaded.", L"Radeon ProRender", MB_OK | MB_ICONEXCLAMATION);

		return 0;
	}

	std::wstring pluginFolder = pluginPath.substr(0, pos);
	std::wstring binFolder = pluginFolder + L"bin\\";

	SetDllDirectory( binFolder.c_str() );

	struct HelperBinaryDescriptor
	{
		enum class BINARY_STATUS { BINARY_OPTIONAL = 0, BINARY_ESSENTIAL = 1 };

		std::wstring  mName;
		BINARY_STATUS mBinaryStatus;
	};

	std::vector<HelperBinaryDescriptor> helperBinaryDescriptors =
	{
		{ L"Tahoe64.dll", HelperBinaryDescriptor::BINARY_STATUS::BINARY_ESSENTIAL },
		{ L"RadeonProRender64.dll", HelperBinaryDescriptor::BINARY_STATUS::BINARY_ESSENTIAL },
		{ L"RprLoadStore64.dll", HelperBinaryDescriptor::BINARY_STATUS::BINARY_ESSENTIAL },

		{ L"RadeonImageFilters.dll", HelperBinaryDescriptor::BINARY_STATUS::BINARY_ESSENTIAL },
	
		{ L"ProRenderGLTF.dll", HelperBinaryDescriptor::BINARY_STATUS::BINARY_OPTIONAL },
	};

	for (const HelperBinaryDescriptor& libraryDescriptor : helperBinaryDescriptors)
	{
		const std::wstring& libraryName = libraryDescriptor.mName;

		HMODULE h = LoadLibrary(libraryName.c_str() );

		if (NULL == h)
		{
			if ( HelperBinaryDescriptor::BINARY_STATUS::BINARY_ESSENTIAL == libraryDescriptor.mBinaryStatus)
			{
				std::wstring message = L"Failed to load " + libraryName + L". Plugin will not be loaded.";
				MessageBox(0, message.c_str(), L"Radeon ProRender", MB_OK | MB_ICONEXCLAMATION);
				SetDllDirectory(NULL);

				return 0;
			}

			if (L"ProRenderGLTF.dll" == libraryName)
			{
				DisableGltfExport();
			}
		}
	}

	SetDllDirectory(NULL);

	LoadLibrary(TEXT("Msftedit.dll"));

	// initialize class descriptors
	gClassInstances.clear();
	gClassInstances.push_back(&FireRender::fireRenderClassDesc);
	gClassInstances.push_back(&FireRender::autoTestingClassDesc);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(DiffuseMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(BlendMtl)::ClassDescInstance);
	//gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(AddMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(MicrofacetMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(ReflectionMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(AmbientOcclusionMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(ArithmMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(InputLUMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(BlendValueMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(RefractionMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(MFRefractionMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(TransparentMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(WardMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(EmissiveMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(FresnelMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::fireRenderMaterialImporterClassDesc);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(ColorMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(AvgMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(OrenNayarMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(DiffuseRefractionMtl)::ClassDescInstance);
	
	if (RPR_API_COMPAT > 0x010000094)
	{
		gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(FresnelSchlickMtl)::ClassDescInstance);
	}
	
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(DisplacementMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(NormalMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(MaterialMtl)::ClassDescInstance);
    gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(UberMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(UberMtlv2)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(UberMtlv3)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(VolumeMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(PbrMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(ShadowCatcherMtl)::ClassDescInstance);
	gClassInstances.push_back(&FireRender::FRMTLCLASSNAME(ColourCorMtl)::ClassDescInstance);

	gClassInstances.push_back(FireRender::GetFireRenderEnvironmentDesc());

	gClassInstances.push_back(FireRender::GetFireRenderAnalyticalSunDesc());

	gClassInstances.push_back(FireRender::GetFireRenderPortalLightDesc());

	gClassInstances.push_back(FireRender::BgManagerMax::GetClassDesc());

	gClassInstances.push_back(FireRender::TmManagerMax::GetClassDesc());

	gClassInstances.push_back(FireRender::CamManagerMax::GetClassDesc());

	gClassInstances.push_back(FireRender::MPManagerMax::GetClassDesc());

	gClassInstances.push_back(FireRender::ScopeManagerMax::GetClassDesc());
	
	gClassInstances.push_back(FireRender::IFRMaterialExporter::GetClassDesc());

	gClassInstances.push_back(FireRender::PRManagerMax::GetClassDesc());

	gClassInstances.push_back(FireRender::FireRenderIESLight::GetClassDesc());

	gClassInstances.push_back(FireRender::FireRenderPhysicalLight::GetClassDesc());

	gClassInstances.push_back(FireRender::RprExporter::GetClassDesc());

	for(int i = 0; i < GetAOVElementClassDescCount(); i++)
	{
		gClassInstances.push_back(&GetAOVElementClassDesc(i));
	}

    return 1;
}

/// Called by 3ds Max immediately before the plugin is unloaded (e.g. when closing the application)
EXPORT_TO_MAX int LibShutdown()
{
    return true;
}

TCHAR* GetString(int id)
{
	static TCHAR buf[1024];

	if( FireRender::fireRenderHInstance )
		return ( LoadString( FireRender::fireRenderHInstance, id, buf, _countof(buf) ) ? buf : NULL );

	return NULL;
}
