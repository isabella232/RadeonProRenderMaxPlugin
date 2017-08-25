#include <array>

#include <Graphics/Utilities/MeshEdgeRenderItem.h>
#include <Graphics/CustomRenderItemHandle.h>
#include <Graphics/RenderNodeHandle.h>
#include <Graphics/IDisplayManager.h>

#include <mouseman.h>
#include <gfx.h>

#include "LookAtTarget.h"

namespace
{
	template<typename T, size_t N>
	constexpr size_t StaticArraySize(const T(&)[N])
	{
		return N;
	}

	class LookAtTargetObjectClassDesc : public ClassDesc
	{
	public:
		static constexpr auto TargetClassName = _T("TARGET");
		static constexpr auto TargetObjectName = TargetClassName;
		static constexpr auto TargetNodeName = TargetClassName;

		int 			IsPublic() override { return 0; }
		void *			Create(BOOL loading = FALSE) override { return new LookAtTarget; }
		const TCHAR *	ClassName() override { return TargetClassName; }
		SClass_ID		SuperClassID() override { return GEOMOBJECT_CLASS_ID; }
		Class_ID		ClassID() override { return Class_ID(TARGET_CLASS_ID, 0); }
		const TCHAR* 	Category() override { return _T("PRIMITIVES"); }
	};

	class MeshCache
	{
	public:
		MeshCache() :
			m_meshBuilt(false)
		{}

		Mesh& GetMesh()
		{
			Build();
			return m_mesh;
		}

		Box3 GetBoundingBox(Matrix3 *tm = nullptr)
		{
			return GetMesh().getBoundingBox(tm);
		}

	private:
		void Build()
		{
			if (m_meshBuilt)
			{
				return;
			}

			constexpr auto h = 20.f;
			constexpr auto s = h;
			constexpr auto hs = s / 2;
			
			constexpr std::array<float, 3> verts[]
			{
				{  0,   0,  0 },
				{ hs,  hs,  h },
				{-hs,  hs,  h },
				{-hs, -hs,  h },
				{ hs, -hs,  h }
			};
			
			constexpr std::array<size_t, 3> sideFaces[]
			{
				{0, 2, 1},
				{0, 3, 2},
				{0, 4, 3},
				{0, 1, 4}
			};
			
			constexpr std::array<size_t, 3> baseFaces[]
			{
				{ 3, 1, 2 },
				{ 1, 3, 4 }
			};
			
			constexpr auto vertsCount = StaticArraySize(verts);
			constexpr auto sideFacesCount = StaticArraySize(sideFaces);
			constexpr auto baseFacesCount = StaticArraySize(baseFaces);
			
			m_mesh.setNumVerts(vertsCount);
			m_mesh.setNumFaces(sideFacesCount + baseFacesCount);
			
			// Set vertices
			for (size_t i = 0; i < vertsCount; ++i)
			{
				auto& v = verts[i];
				m_mesh.setVert(i, v[0], v[1], v[2]);
			}
			
			size_t nextSmGroup = 0;
			size_t nextFaceIndex = 0;
			
			// Set side faces
			for (size_t i = 0; i < sideFacesCount; ++i)
			{
				auto& f = sideFaces[i];
				auto& meshFace = m_mesh.faces[nextFaceIndex++];
			
				meshFace.setVerts(f[0], f[1], f[2]);
				meshFace.setSmGroup(1 << (nextSmGroup++));
				meshFace.setEdgeVisFlags(1, 1, 1);
			}
			
			// Set base faces
			for (size_t i = 0; i < baseFacesCount; ++i)
			{
				auto& f = baseFaces[i];
				auto& meshFace = m_mesh.faces[nextFaceIndex++];
			
				meshFace.setVerts(f[0], f[1], f[2]);
				meshFace.setSmGroup(1 << nextSmGroup);
				meshFace.setEdgeVisFlags(0, 1, 1);
			}
			
			m_mesh.buildNormals();
			m_mesh.EnableEdgeList(1);

			m_meshBuilt = true;
		}

		Mesh m_mesh;
		bool m_meshBuilt;
	};

	static LookAtTargetObjectClassDesc lookAtTargetObjDesc;
	static MeshCache meshCache;
}

void LookAtTarget::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev){}
void LookAtTarget::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next ){}
const TCHAR* LookAtTarget::GetObjectName() { return LookAtTargetObjectClassDesc::TargetObjectName; }

class LookAtTarget::CreateCallBack :
	public CreateMouseCallBack
{
	LookAtTarget *ob;
public:
	int proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat)
	{
		if (!vpt || !vpt->IsAlive())
		{
			DbgAssert(!_T("Invalid viewport!"));
			return FALSE;
		}


		switch (msg)
		{
			case MOUSE_POINT:
			case MOUSE_MOVE:
			{
				Point3 c;

				switch (point)
				{
				case 0:
					c = vpt->GetPointOnCP(m);
					mat.SetTrans(c);
					return CREATE_STOP;
				}
			}
			break;

			case MOUSE_ABORT:
				return CREATE_ABORT;
		}

		return TRUE;
	}

	void SetObj(LookAtTarget *obj) { ob = obj; }

	static CreateCallBack* GetInstance()
	{
		static CreateCallBack instance;
		return &instance;
	}
};

CreateMouseCallBack* LookAtTarget::GetCreateMouseCallBack()
{
	auto instance = CreateCallBack::GetInstance();
	instance->SetObj(this);
	return instance;
}

void LookAtTarget::AddRenderitem(
	const MaxSDK::Graphics::UpdateDisplayContext& updateDisplayContext,
	MaxSDK::Graphics::UpdateNodeContext& nodeContext,
	MaxSDK::Graphics::IRenderItemContainer& targetRenderItemContainer)
{}

unsigned long LookAtTarget::GetObjectDisplayRequirement() const
{
	return 1;
}

bool LookAtTarget::PrepareDisplay(const MaxSDK::Graphics::UpdateDisplayContext& displayContext)
{
	return true;
}

bool LookAtTarget::UpdatePerNodeItems(
	const MaxSDK::Graphics::UpdateDisplayContext& /*updateDisplayContext*/,
	MaxSDK::Graphics::UpdateNodeContext& /*nodeContext*/,
	MaxSDK::Graphics::IRenderItemContainer& /*targetRenderItemContainer*/)
{
	return true;
}

LookAtTarget::LookAtTarget() = default;
LookAtTarget::~LookAtTarget() = default;

void LookAtTarget::GetMat(TimeValue t, INode* inode, ViewExp& vpt, Matrix3& tm)
{
	if ( !vpt.IsAlive() )
	{
		tm.Zero();
		return;
	}
	
	tm = inode->GetObjectTM(t);
	tm.NoScale();
	float scaleFactor = vpt.NonScalingObjectSize() * vpt.GetVPWorldWidth(tm.GetTrans()) / 360.0f;
	
	if (scaleFactor!=(float)1.0)
		tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
}

void LookAtTarget::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
{
	box = meshCache.GetBoundingBox(tm);
}

BOOL LookAtTarget::HasViewDependentBoundingBox()
{
	return true;
}

void LookAtTarget::DeleteThis()
{
	delete this;
}

Class_ID LookAtTarget::ClassID()
{
	return Class_ID(TARGET_CLASS_ID, 0);
}

void LookAtTarget::GetClassName(TSTR& s)
{
	s = LookAtTargetObjectClassDesc::TargetClassName;
}

int LookAtTarget::IsKeyable()
{
	return 1;
}

LRESULT CALLBACK LookAtTarget::TrackViewWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void LookAtTarget::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{
	if ( ! vpt || ! vpt->IsAlive() )
	{
		box.Init();
		return;
	}
	
	Matrix3 m = inode->GetObjectTM(t);
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(m.GetTrans())/(float)360.0;
	box = meshCache.GetBoundingBox();
	box.Scale(scaleFactor);
}

void LookAtTarget::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box)
{
	if ( !vpt || !vpt->IsAlive() )
	{
		box.Init();
		return;
	}

	auto& mesh = meshCache.GetMesh();

	int i,nv;
	Matrix3 m;
	GetMat(t,inode,*vpt,m);
	nv = mesh.getNumVerts();
	box.Init();
	
	for (i=0; i<nv; i++) 
		box += m*mesh.getVert(i);
}

int LookAtTarget::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
{
	if ( ! vpt || ! vpt->IsAlive() )
	{
		FASSERT(!_T("Invalid viewport!"));
		return FALSE;
	}
	
	HitRegion hitRegion;
	MakeHitRegion(hitRegion,type,crossing,4,p);

	DWORD savedLimits;
	auto gw = vpt->getGW();
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);

	Matrix3 m;
	GetMat(t,inode,*vpt,m);
	gw->setTransform(m);

	if (meshCache.GetMesh().select(gw, gw->getMaterial(), &hitRegion, flags & HIT_ABORTONHIT))
	{
		return TRUE;
	}

	gw->setRndLimits(savedLimits);

	return FALSE;

#if 0
	gw->setHitRegion(&hitRegion);
	gw->clearHitCode();
	gw->fWinMarker(&pt, HOLLOW_BOX_MRKR);
	return gw->checkHitCode();
#endif
}

void LookAtTarget::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt)
{	
	if ( !vpt || ! vpt->IsAlive() )
	{
		DbgAssert(!_T("Invalid viewport!"));
		return;
	}

	// Make sure the vertex priority is active and at least as important as the best snap so far
	if (snap->vertPriority > 0 && snap->vertPriority <= snap->priority)
	{
		Matrix3 tm = inode->GetObjectTM(t);	
		GraphicsWindow *gw = vpt->getGW();	
		gw->setTransform(tm);

		Matrix3 invPlane = Inverse(snap->plane);

		Point2 fp = Point2((float)p->x, (float)p->y);
		IPoint3 screen3;
		Point2 screen2;

		Point3 thePoint(0,0,0);
		// If constrained to the plane, make sure this point is in it!
		if (snap->snapType == SNAP_2D || snap->flags & SNAP_IN_PLANE)
		{
			Point3 test = thePoint * tm * invPlane;
			if(fabs(test.z) > 0.0001)	// Is it in the plane (within reason)?
				return;
		}

		gw->wTransPoint(&thePoint,&screen3);
		screen2.x = (float)screen3.x;
		screen2.y = (float)screen3.y;
		// Are we within the snap radius?
		int len = (int)Length(screen2 - fp);

		if (len <= snap->strength)
		{
			// Is this priority better than the best so far?
			if(snap->vertPriority < snap->priority)
			{
				snap->priority = snap->vertPriority;
				snap->bestWorld = thePoint * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
			}
			else if(len < snap->bestDist)
			{
				snap->priority = snap->vertPriority;
				snap->bestWorld = thePoint * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
			}
		}
	}
}

int LookAtTarget::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
{
	if (!vpt || !vpt->IsAlive())
	{
		DbgAssert(!_T("Invalid viewport!"));
		return FALSE;
	}
	
	if (MaxSDK::Graphics::IsRetainedModeEnabled() &&
		vpt != nullptr &&
		vpt->GetViewCamera() != nullptr &&
		vpt->GetViewCamera()->GetTarget() == inode)
	{
		return 0;
	}

	Matrix3 m;
	auto gw = vpt->getGW();
	GetMat(t,inode,*vpt,m);
	gw->setTransform(m);

	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL| (rlim&GW_Z_BUFFER) );
	
	if (inode->Selected())
	{
		gw->setColor(LINE_COLOR, GetSelColor());
	}
	else if (!inode->IsFrozen() && !inode->Dependent() && inode->GetLookatNode())
	{
		const ObjectState& os = inode->GetLookatNode()->EvalWorldState(t);
		Object* ob = os.obj;

		if (ob != nullptr && ( ( ob->SuperClassID() == LIGHT_CLASS_ID ) || ( ob->SuperClassID() == CAMERA_CLASS_ID ) ) )
		{													
			Color color(inode->GetWireColor());
			gw->setColor( LINE_COLOR, color );
		}
		else
			gw->setColor( LINE_COLOR, GetUIColor(COLOR_CAMERA_OBJ)); // default target color, just use camera targ color
	}

	meshCache.GetMesh().render( gw, gw->getMaterial(), NULL, COMP_ALL);	
	gw->setRndLimits(rlim);
	
	return 0;
}

int LookAtTarget::IntersectRay(TimeValue t, Ray& r, float& at)
{
	return 0;
}

// This is only called if the object MAKES references to other things.
RefResult LookAtTarget::NotifyRefChanged(
	const Interval& changeInt, RefTargetHandle hTarget,
     PartID& partID, RefMessage message, BOOL propagate ) 
{
	switch (message)
	{
	case REFMSG_NODE_WIRECOLOR_CHANGED:
		Beep( 1000, 500 );
	}

	return REF_SUCCEED;
}

ObjectState LookAtTarget::Eval(TimeValue time)
{
	return ObjectState(this);
}

void LookAtTarget::InitNodeName(TSTR& s)
{
	s = LookAtTargetObjectClassDesc::TargetNodeName;
}

int LookAtTarget::UsesWireColor()
{
	return 1;
}

int LookAtTarget::IsRenderable()
{
	return 0;
}

RefTargetHandle LookAtTarget::Clone(RemapDir& remap)
{
	LookAtTarget* newObject = new LookAtTarget();
	BaseClone(this, newObject, remap);
	
	return newObject;
}

IOResult LookAtTarget::Save(ISave* isave)
{
	return IO_OK;
}

IOResult LookAtTarget::Load(ILoad* iload)
{
	return IO_OK;
}
