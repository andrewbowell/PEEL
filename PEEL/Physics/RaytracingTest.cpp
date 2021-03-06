///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RaytracingTest.h"
#include "RaytracingWindow.h"
#include "Pint.h"
#include "PintSQ.h"
#include "Camera.h"
#include "SourceRay.h"

static inline_ void SetBackgroundColor(RGBAPixel& pixel)
{
	pixel.R = pixel.G = pixel.B = 51;
	pixel.A = 255;
}

static inline_ void ComputeShading(RGBAPixel& color, const PintRaycastHit& hit,
								   const Point& light_dir, const Point& light_color)
{
//	float LDotN = fabsf(light_dir|hit.mNormal);
	float LDotN = light_dir|hit.mNormal;
	if(LDotN<0.0f)
		LDotN = 0.0f;

	color.R = ubyte(light_color.x * LDotN);
	color.G = ubyte(light_color.y * LDotN);
	color.B = ubyte(light_color.z * LDotN);

	color.A = 255;
}

/*static*/ Point ComputeWorldRay(const GLint viewPort[4], const GLdouble modelMatrix[16], const GLdouble projMatrix[16], int xs, int ys)
{
	ys = viewPort[3] - ys - 1;
	GLdouble wx0, wy0, wz0;
	gluUnProject((GLdouble) xs, (GLdouble) ys, 0.0, modelMatrix, projMatrix, viewPort, &wx0, &wy0, &wz0);
	GLdouble wx1, wy1, wz1;
	gluUnProject((GLdouble) xs, (GLdouble) ys, 1.0, modelMatrix, projMatrix, viewPort, &wx1, &wy1, &wz1);
	Point tmp(float(wx1-wx0), float(wy1-wy0), float(wz1-wz0));
	tmp.Normalize();
	return tmp;
}

udword RaytracingTest(Picture& pic, Pint& pint, udword& total_time, udword screen_width, udword screen_height, udword nb_rays, float max_dist)
{
//	max_dist = MAX_FLOAT;
	udword TotalTime = 0;
	udword Nb=0;
	const Point origin = GetCameraPos();

	Point LightColor = pint.GetMainColor();
	LightColor *= 255.0f;

	const udword RAYTRACING_RENDER_WIDTH = nb_rays;
	const udword RAYTRACING_RENDER_HEIGHT = nb_rays;

	ASSERT(pic.GetWidth()==RAYTRACING_RENDER_WIDTH);
	ASSERT(pic.GetHeight()==RAYTRACING_RENDER_HEIGHT);
	RGBAPixel* Pixels = pic.GetPixels();

	const float fScreenWidth = float(screen_width)/float(RAYTRACING_RENDER_WIDTH);
	const float fScreenHeight = float(screen_height)/float(RAYTRACING_RENDER_HEIGHT);

	Point LightDir(1.0f, 1.0f, 0.5f);
	LightDir.Normalize();

	if(0)
	{
		FILE* fp = fopen("f:\\tmp\\rays.bin", "wb");
		if(fp)
		{
			const udword TotalNbRays = RAYTRACING_RENDER_HEIGHT * RAYTRACING_RENDER_WIDTH;
			fwrite(&TotalNbRays, sizeof(udword), 1, fp);

			for(udword j=0;j<RAYTRACING_RENDER_HEIGHT;j++)
			{
				const udword yi = udword(fScreenHeight*float(j));
				for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
				{
					const udword xi = udword(fScreenWidth*float(i));

					const Point dir = ComputeWorldRay(xi, yi);

					Source1_Ray_t R;
					R.m_Start.x			= origin.x;
					R.m_Start.y			= origin.y;
					R.m_Start.z			= origin.z;
					R.m_Start.w			= 0.0f;
					R.m_Delta.x			= dir.x * max_dist;
					R.m_Delta.y			= dir.y * max_dist;
					R.m_Delta.z			= dir.z * max_dist;
					R.m_Delta.w			= 0.0f;
					R.m_StartOffset.x	= 0.0f;
					R.m_StartOffset.y	= 0.0f;
					R.m_StartOffset.z	= 0.0f;
					R.m_StartOffset.w	= 0.0f;
					R.m_Extents.x		= 0.0f;
					R.m_Extents.y		= 0.0f;
					R.m_Extents.z		= 0.0f;
					R.m_Extents.w		= 0.0f;
					R.m_pDummy			= null;
					R.m_IsRay			= true;
					R.m_IsSwept			= false;

					fwrite(&R, sizeof(Source1_Ray_t), 1, fp);
				}
			}
			fclose(fp);
		}
	}

	{
		__declspec(align(16))	PintRaycastHit Hits[RAYTRACING_MAX_RENDER_SIZE];
		__declspec(align(16))	PintRaycastData RaycastData[RAYTRACING_MAX_RENDER_SIZE];

		for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
		{
			RaycastData[i].mOrigin = origin;
			RaycastData[i].mMaxDist = max_dist;
		}

GLint viewPort[4];
GLdouble modelMatrix[16];
GLdouble projMatrix[16];
glGetIntegerv(GL_VIEWPORT, viewPort);
glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

//#define COLLECT_BLACK_PIXELS
#ifdef COLLECT_BLACK_PIXELS
		Vertices BlackRays;
#endif
		for(udword j=0;j<RAYTRACING_RENDER_HEIGHT;j++)
		{
			const udword yi = udword(fScreenHeight*float(j));
			for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
			{
				const udword xi = udword(fScreenWidth*float(i));
//				RaycastData[i].mDir = ComputeWorldRay(xi, yi);
RaycastData[i].mDir = ComputeWorldRay(viewPort, modelMatrix, projMatrix, xi, yi);
				Hits[i].mObject = null;
			}

			udword Time;
			StartProfile(Time);
				pint.BatchRaycasts(pint.mSQHelper->GetThreadContext(), RAYTRACING_RENDER_WIDTH, Hits, RaycastData);
			EndProfile(Time);
			TotalTime += Time;

			for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
			{
				if(Hits[i].mObject)
				{
					Nb++;
					ComputeShading(*Pixels, Hits[i], LightDir, LightColor);
				}
				else
				{
					SetBackgroundColor(*Pixels);
#ifdef COLLECT_BLACK_PIXELS
					BlackRays.AddVertex(RaycastData[i].mOrigin);
					BlackRays.AddVertex(RaycastData[i].mDir);
#endif
				}
				Pixels++;
			}
		}
#ifdef COLLECT_BLACK_PIXELS
		FILE* fp = fopen("f:\\tmp\\black_rays.bin", "wb");
		if(fp)
		{
			const udword TotalNbRays = BlackRays.GetNbVertices()/2;
			fwrite(&TotalNbRays, sizeof(udword), 1, fp);

			const Point* Data = BlackRays.GetVertices();

			for(udword i=0;i<TotalNbRays;i++)
			{
				Source1_Ray_t R;
				R.m_Start.x			= Data[i*2].x;
				R.m_Start.y			= Data[i*2].y;
				R.m_Start.z			= Data[i*2].z;
				R.m_Start.w			= 0.0f;
				R.m_Delta.x			= Data[i*2+1].x * max_dist;
				R.m_Delta.y			= Data[i*2+1].y * max_dist;
				R.m_Delta.z			= Data[i*2+1].z * max_dist;
				R.m_Delta.w			= 0.0f;
				R.m_StartOffset.x	= 0.0f;
				R.m_StartOffset.y	= 0.0f;
				R.m_StartOffset.z	= 0.0f;
				R.m_StartOffset.w	= 0.0f;
				R.m_Extents.x		= 0.0f;
				R.m_Extents.y		= 0.0f;
				R.m_Extents.z		= 0.0f;
				R.m_Extents.w		= 0.0f;
				R.m_pDummy			= null;
				R.m_IsRay			= true;
				R.m_IsSwept			= false;

				fwrite(&R, sizeof(Source1_Ray_t), 1, fp);
			}
			fclose(fp);
		}
#endif
		total_time = TotalTime;
	}
	return Nb;
}







struct RaytracingParams
{
	Pint*				mPint;
	PintSQThreadContext	mContext;
	RGBAPixel*			mPixels;

	Point				mOrigin;
	Point				mLightDir;
	Point				mLightColor;

	udword				mNbToGo;
	const Point*		mDirs;
	float				mMaxDist;

	udword				RAYTRACING_RENDER_WIDTH;
	udword				RAYTRACING_RENDER_HEIGHT;

	udword				mNb;
	udword				mTotalTime;
};

void ThreadSetup();

static int gRaytracingThread(void* user_data)
{
	ThreadSetup();

	RaytracingParams* __restrict Params = (RaytracingParams* __restrict)user_data;

	RGBAPixel* __restrict Dest = Params->mPixels;

	const udword RAYTRACING_RENDER_WIDTH = Params->RAYTRACING_RENDER_WIDTH;
	const udword RAYTRACING_RENDER_HEIGHT = Params->RAYTRACING_RENDER_HEIGHT;

	udword TotalTime = 0;
	udword Nb = 0;

	{
		__declspec(align(16))	PintRaycastHit Hits[RAYTRACING_MAX_RENDER_SIZE];
		__declspec(align(16))	PintRaycastData RaycastData[RAYTRACING_MAX_RENDER_SIZE];

		{
			const Point Origin = Params->mOrigin;
			for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
			{
				RaycastData[i].mOrigin = Origin;
				RaycastData[i].mMaxDist = Params->mMaxDist;
			}
		}

		udword NbToGo = Params->mNbToGo;
		const Point* __restrict Dirs = Params->mDirs;
		const Point LightDir = Params->mLightDir;
		const Point LightColor = Params->mLightColor;
		while(NbToGo)
		{
			NbToGo -= RAYTRACING_RENDER_WIDTH;
			for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
			{
				RaycastData[i].mDir = *Dirs++;
				Hits[i].mObject = null;
			}

//			udword Time;
//			StartProfile(Time);
				Params->mPint->BatchRaycasts(Params->mContext, RAYTRACING_RENDER_WIDTH, Hits, RaycastData);
//			EndProfile(Time);
//			TotalTime += Time;

			for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
			{
				if(Hits[i].mObject)
				{
					Nb++;
					ComputeShading(*Dest, Hits[i], LightDir, LightColor);
				}
				else
				{
					SetBackgroundColor(*Dest);
				}
				Dest++;
			}
		}
	}

	Params->mTotalTime = TotalTime;
	Params->mNb = Nb;

	return 1;	// Written as "status" member
}

udword RaytracingTestMT(Picture& pic, Pint& pint, udword& total_time, udword screen_width, udword screen_height, udword nb_rays, float max_dist)
{
	const Point origin = GetCameraPos();

	Point LightColor = pint.GetMainColor();
	LightColor *= 255.0f;

	const udword RAYTRACING_RENDER_WIDTH = nb_rays;
	const udword RAYTRACING_RENDER_HEIGHT = nb_rays;

	ASSERT(pic.GetWidth()==RAYTRACING_RENDER_WIDTH);
	ASSERT(pic.GetHeight()==RAYTRACING_RENDER_HEIGHT);
	RGBAPixel* Pixels = pic.GetPixels();

	const float fScreenWidth = float(screen_width)/float(RAYTRACING_RENDER_WIDTH);
	const float fScreenHeight = float(screen_height)/float(RAYTRACING_RENDER_HEIGHT);

	Point LightDir(1.0f, 1.0f, 0.5f);
	LightDir.Normalize();

	const udword NbPixels = RAYTRACING_RENDER_WIDTH*RAYTRACING_RENDER_HEIGHT;

	Point* Dirs = ICE_NEW(Point)[NbPixels];
	{
GLint viewPort[4];
GLdouble modelMatrix[16];
GLdouble projMatrix[16];
glGetIntegerv(GL_VIEWPORT, viewPort);
glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

		Point* Dest = Dirs;
		for(udword j=0;j<RAYTRACING_RENDER_HEIGHT;j++)
		{
			const udword yi = udword(fScreenHeight*float(j));
			for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
			{
				const udword xi = udword(fScreenWidth*float(i));
//				const Point dir = ComputeWorldRay(xi, yi);
const Point dir = ComputeWorldRay(viewPort, modelMatrix, projMatrix, xi, yi);
				*Dest++ = dir;
			}
		}
	}

	udword Time;
	RaytracingParams Params[4];
	if(0)
	{
		for(udword i=0;i<1;i++)
		{
			Params[i].RAYTRACING_RENDER_WIDTH	= nb_rays;
			Params[i].RAYTRACING_RENDER_HEIGHT	= nb_rays;

			Params[i].mPint			= &pint;
			Params[i].mPixels		= Pixels + i*(NbPixels/4);
			Params[i].mOrigin		= origin,
			Params[i].mLightDir		= LightDir;
			Params[i].mLightColor	= LightColor * (1.0 - float(i)*0.2f);
			Params[i].mNb			= 0;
			Params[i].mTotalTime	= 0;
			Params[i].mNbToGo		= NbPixels/4;
	//Params[i].mNbToGo		= NbPixels;
			Params[i].mDirs			= Dirs + i*(NbPixels/4);
			Params[i].mMaxDist		= max_dist;
		}

		StartProfile(Time);

			IceThread* Thread1 = CreateThread(gRaytracingThread, &Params[0]);
			WaitThread(Thread1, null);

		EndProfile(Time);
	}
	else
	{
		for(udword i=0;i<4;i++)
		{
			Params[i].RAYTRACING_RENDER_WIDTH	= nb_rays;
			Params[i].RAYTRACING_RENDER_HEIGHT	= nb_rays;

			Params[i].mPint			= &pint;
			Params[i].mContext		= pint.CreateSQThreadContext();
			Params[i].mPixels		= Pixels + i*(NbPixels/4);
			Params[i].mOrigin		= origin,
			Params[i].mLightDir		= LightDir;
			Params[i].mLightColor	= LightColor * (1.0 - float(i)*0.2f);
			Params[i].mNb			= 0;
			Params[i].mTotalTime	= 0;
			Params[i].mNbToGo		= NbPixels/4;
	//Params[i].mNbToGo		= NbPixels;
			Params[i].mDirs			= Dirs + i*(NbPixels/4);
			Params[i].mMaxDist		= max_dist;
		}

		StartProfile(Time);
		{
			IceThread* Thread1 = CreateThread(gRaytracingThread, &Params[0]);
			IceThread* Thread2 = CreateThread(gRaytracingThread, &Params[1]);
			IceThread* Thread3 = CreateThread(gRaytracingThread, &Params[2]);
//			IceThread* Thread4 = CreateThread(gRaytracingThread, &Params[3]);
				gRaytracingThread(&Params[3]);
			WaitThread(Thread1, null);
			WaitThread(Thread2, null);
			WaitThread(Thread3, null);
//			WaitThread(Thread4, null);
		}
		EndProfile(Time);
	}

	DELETEARRAY(Dirs);

	total_time = 0;
	udword Nb = 0;
	for(udword i=0;i<4;i++)
	{
		pint.ReleaseSQThreadContext(Params[i].mContext);

		total_time += Params[i].mTotalTime;
		Nb += Params[i].mNb;
	}
//total_time = T;
total_time = Time;
	return Nb;
}
