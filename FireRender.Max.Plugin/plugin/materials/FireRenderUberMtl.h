#pragma once

#include "FireRenderMtlBase.h"

FIRERENDER_NAMESPACE_BEGIN;

enum FRUberMtl_TexmapId {
	FRUBERMTL_TEXMAP_DIFFLEVEL = 0,
	FRUBERMTL_TEXMAP_DIFFCOLOR,
	FRUBERMTL_TEXMAP_DIFFNORMAL,
	FRUBERMTL_TEXMAP_GLOSSYCOLOR,
	FRUBERMTL_TEXMAP_GLOSSYROUGHNESS_X,
	FRUBERMTL_TEXMAP_GLOSSYROUGHNESS_Y,
	FRUBERMTL_TEXMAP_GLOSSYROTATION,
	FRUBERMTL_TEXMAP_GLOSSYNORMAL,
	FRUBERMTL_TEXMAP_CCCOLOR,
	FRUBERMTL_TEXMAP_CCNORMAL,
	FRUBERMTL_TEXMAP_REFRCOLOR,
	FRUBERMTL_TEXMAP_REFRROUGHNESS,
	FRUBERMTL_TEXMAP_REFRNORMAL,
	FRUBERMTL_TEXMAP_TRANSPLEVEL,
	FRUBERMTL_TEXMAP_TRANSPCOLOR,
	FRUBERMTL_TEXMAP_DISPLACEMENT,
	// volume
	FRUBERMTL_TEXMAP_COLOR,
	FRUBERMTL_TEXMAP_DISTANCE,
	FRUBERMTL_TEXMAP_EMISSION
};

enum FRUberMtl_ParamID : ParamID {
	FRUBERMTL_DIFFLEVEL = 1000,
	FRUBERMTL_DIFFLEVEL_TEXMAP,
	FRUBERMTL_DIFFCOLOR,
	FRUBERMTL_DIFFCOLOR_TEXMAP,
	FRUBERMTL_DIFFNORMAL,

	FRUBERMTL_GLOSSYUSE,
	FRUBERMTL_GLOSSYCOLOR,
	FRUBERMTL_GLOSSYCOLOR_TEXMAP,
	FRUBERMTL_GLOSSYROTATION,
	FRUBERMTL_GLOSSYROTATION_TEXMAP,
	FRUBERMTL_GLOSSYROUGHNESS_X,
	FRUBERMTL_GLOSSYROUGHNESS_X_TEXMAP,
	FRUBERMTL_GLOSSYROUGHNESS_Y,
	FRUBERMTL_GLOSSYROUGHNESS_Y_TEXMAP,
	FRUBERMTL_GLOSSYIOR,
	FRUBERMTL_GLOSSYNORMAL,

	FRUBERMTL_CCUSE,
	FRUBERMTL_CCCOLOR,
	FRUBERMTL_CCCOLOR_TEXMAP,
	FRUBERMTL_CCIOR,
	FRUBERMTL_CCNORMAL,

	FRUBERMTL_REFRROUGHNESS,
	FRUBERMTL_REFRCOLOR,
	FRUBERMTL_REFRCOLOR_TEXMAP,
	FRUBERMTL_REFRROUGHNESS_TEXMAP,
	FRUBERMTL_REFRIOR,
	FRUBERMTL_REFRNORMAL,

	FRUBERMTL_TRANSPLEVEL,
	FRUBERMTL_TRANSPLEVEL_TEXMAP,
	FRUBERMTL_TRANSPCOLOR,
	FRUBERMTL_TRANSPCOLOR_TEXMAP,

	FRUBERMTL_DISPLACEMENT,
	FRUBERMTL_FRUBERCAUSTICS,
	FRUBERMTL_FRUBERSHADOWCATCHER,

	// volume
	FRUBERMTL_USEVOLUME,
	FRUBERMTL_COLOR,
	FRUBERMTL_COLORTEXMAP,
	FRUBERMTL_DISTANCE,
	FRUBERMTL_DISTANCETEXMAP,
	FRUBERMTL_EMISSIONMULTIPLIER,
	FRUBERMTL_EMISSIONCOLOR,
	FRUBERMTL_EMISSIONCOLORTEXMAP,
	FRUBERMTL_SCATTERINGDIRECTION,
	FRUBERMTL_MULTISCATTERING,

	// invert transparency map (to use opacity maps)
	FRUBERMTL_INVERTTRANSPMAP
};

BEGIN_DECLARE_FRMTLCLASSDESC(UberMtl, L"RPR Uber Material", FIRERENDER_UBERMTL_CID)
END_DECLARE_FRMTLCLASSDESC()

BEGIN_DECLARE_FRMTL(UberMtl)
public:
	virtual Color GetDiffuse(int mtlNum, BOOL backFace) override
	{
		return GetFromPb<Color>(pblock, FRUBERMTL_DIFFCOLOR);
	}
	frw::Shader getVolumeShader(const TimeValue t, MaterialParser& mtlParser, INode* node);
END_DECLARE_FRMTL(UberMtl)

FIRERENDER_NAMESPACE_END;