//
//	OculusANE.cpp
//	Oculus Rift Adobe Native Extension for Windows
//
//	Date: May 28, 2013
//	Author: Micheal MacLean
//	Copyright: (c) 2013 New Nihilist. All rights reserved
//
//	Heavily modified June 5, 2013 to work with jonathanhart/oculus-ane
//

#include "OculusANE.h"

#include <stdlib.h>
#include <iostream>
#include <conio.h>

using namespace OVR;
using namespace std;

extern "C" {

	ovrHmd HMD = NULL;
	ovrEyeRenderDesc	eyeRenderDesc[2];
	ovrRecti			eyeRenderViewport[2];
	ovrVector2f			UVScaleOffset[2][2];
	Sizei				renderTargetSize;
	ovrFovPort			eyeFov[2];
	
	FREObject isSupported(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
		FREObject result;

		uint32_t isSupportedSwitch = 1;

		if (HMD == NULL) {
			isSupportedSwitch = 0;
		}

		FRENewObjectFromBool(isSupportedSwitch, &result);

		return result;
	}

	FREObject getResolution(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
	{
		FREObject resolutionResult;
		FRENewObject((const uint8_t*)"Vector.<Number>", 0, NULL, &resolutionResult, NULL);
		FRESetArrayLength(&resolutionResult, 2);

		ovrSizei size = HMD->Resolution;

		// get an element at index
		FREObject xVal;
		double x = static_cast<double>(size.w);
		FRENewObjectFromDouble(x, &xVal);
		FRESetArrayElementAt(resolutionResult, 0, xVal);

		FREObject yVal;
		double y = static_cast<double>(size.h);
		FRENewObjectFromDouble(y, &yVal);
		FRESetArrayElementAt(resolutionResult, 1, yVal);

		return resolutionResult;
	}




	FREObject getCameraPosition(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
	{
		FREObject positionResult;
		FRENewObject((const uint8_t*)"Vector.<Number>", 0, NULL, &positionResult, NULL);
		FRESetArrayLength(&positionResult, 6);

		static ovrPosef eyeRenderPose[2];
		for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)
		{
			ovrEyeType eye = HMD->EyeRenderOrder[eyeIndex];
			eyeRenderPose[eye] = ovrHmd_GetEyePose(HMD, eye);

			FREObject xVal;
			double x = static_cast<double>(eyeRenderPose[eye].Position.x);
			FRENewObjectFromDouble(x, &xVal);
			FRESetArrayElementAt(positionResult, (eyeIndex * 3) + 0, xVal);

			FREObject yVal;
			double y = static_cast<double>(eyeRenderPose[eye].Position.y);
			FRENewObjectFromDouble(y, &yVal);
			FRESetArrayElementAt(positionResult, (eyeIndex * 3) + 1, yVal);

			FREObject zVal;
			double z = static_cast<double>(eyeRenderPose[eye].Position.z);
			FRENewObjectFromDouble(z, &zVal);
			FRESetArrayElementAt(positionResult, (eyeIndex * 3) + 2, zVal);
		}


		return positionResult;
	}

	FREObject getCameraQuaternion(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
	{
		FREObject cameraQuaternionResult;
		FRENewObject((const uint8_t*)"Vector.<Number>", 0, NULL, &cameraQuaternionResult, NULL);
		FRESetArrayLength(&cameraQuaternionResult, 4);

		// Query the HMD for the current tracking state.
		ovrTrackingState ts = ovrHmd_GetTrackingState(HMD, ovr_GetTimeInSeconds());
		//   if ((ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
		//   {
		ovrQuatf quaternion = ts.HeadPose.ThePose.Orientation;


		// NSLog(@"getCameraQuaternion %f %f %f %f", quaternion.x, quaternion.y, quaternion.z, quaternion.w);

		// get an element at index
		FREObject xVal;
		double x = static_cast<double>(quaternion.x);
		FRENewObjectFromDouble(x, &xVal);
		FRESetArrayElementAt(cameraQuaternionResult, 0, xVal);

		FREObject yVal;
		double y = static_cast<double>(quaternion.y);
		FRENewObjectFromDouble(y, &yVal);
		FRESetArrayElementAt(cameraQuaternionResult, 1, yVal);

		FREObject zVal;
		double z = static_cast<double>(quaternion.z);
		FRENewObjectFromDouble(z, &zVal);
		FRESetArrayElementAt(cameraQuaternionResult, 2, zVal);

		FREObject wVal;
		double w = static_cast<double>(quaternion.w);
		FRENewObjectFromDouble(w, &wVal);
		FRESetArrayElementAt(cameraQuaternionResult, 3, wVal);

		// NSLog(@"Quat Vals: %f,%f,%f,%f", quaternion.x, quaternion.y, quaternion.z, quaternion.w);

		//     }


		return cameraQuaternionResult;
	}



	FREObject getHMDInfo(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
	{
		FREObject result;
		FRENewObject((const uint8_t*)"Object", 0, NULL, &result, NULL);

		ovrSizei size = HMD->Resolution;
		/*
		ovrHmd_GetRenderDesc(hmd, <#ovrEyeType eyeType#>, <#ovrFovPort fov#>)
		FREObject hScreenSize;
		FRENewObjectFromDouble(static_cast<double>(size.w), &hScreenSize);
		FRESetObjectProperty(result, (const uint8_t*)"HScreenSize", hScreenSize, NULL);

		FREObject vScreenSize;
		FRENewObjectFromDouble(static_cast<double>(size.h), &vScreenSize);
		FRESetObjectProperty(result, (const uint8_t*)"VScreenSize", vScreenSize, NULL);

		FREObject eyeToScreenDistance;
		FRENewObjectFromDouble(static_cast<double>(hmd->DefaultEyeFov->), &eyeToScreenDistance);
		FRESetObjectProperty(result, (const uint8_t*)"EyeToScreenDistance", eyeToScreenDistance, NULL);

		FREObject lensSeparationDistance;
		FRENewObjectFromDouble(static_cast<double>(info.LensSeparationDistance), &lensSeparationDistance);
		FRESetObjectProperty(result, (const uint8_t*)"LensSeparationDistance", lensSeparationDistance, NULL);

		FREObject interPupillaryDistance;
		FRENewObjectFromDouble(static_cast<double>(info.InterpupillaryDistance), &interPupillaryDistance);
		FRESetObjectProperty(result, (const uint8_t*)"InterpupillaryDistance", interPupillaryDistance, NULL);

		FREObject kDistortion;
		FRENewObject((const uint8_t*)"Vector.<Number>", 0, NULL, &kDistortion, nil);
		FRESetArrayLength(kDistortion, 4);
		for(int i=0; i<4; i++) {
		FREObject kValue;
		FRENewObjectFromDouble(static_cast<double>(info.DistortionK[i]), &kValue);
		FRESetArrayElementAt(kDistortion, i, kValue);
		}

		FRESetObjectProperty(result, (const uint8_t*)"DistortionK", kDistortion, NULL);
		*/
		return result;
	}

	void OculusANE_ContextInitializer(void* extData, const uint8_t* ctxType, FREContext ctx, uint32_t* numFunctionsToSet, const FRENamedFunction** functionsToSet){

		int functions = 5;

		*numFunctionsToSet = functions;

		FRENamedFunction* func = (FRENamedFunction*)malloc(sizeof(FRENamedFunction)* functions);

		func[0].name = (const uint8_t*)"isSupported";
		func[0].functionData = NULL;
		func[0].function = &isSupported;

		func[1].name = (const uint8_t*)"getCameraQuaternion";
		func[1].functionData = NULL;
		func[1].function = &getCameraQuaternion;

		func[2].name = (const uint8_t*)"getHMDInfo";
		func[2].functionData = NULL;
		func[2].function = &getHMDInfo;

		func[3].name = (const uint8_t*)"getOculusResolution";
		func[3].functionData = NULL;
		func[3].function = &getResolution;

		func[4].name = (const uint8_t*)"getCameraPosition";
		func[4].functionData = NULL;
		func[4].function = &getCameraPosition;
		*functionsToSet = func;

		cout << "Initialized Native Extension\n";

		ovr_Initialize();
		HMD = ovrHmd_Create(0);
		if (!HMD)
		{
			cout << "Oculus Rift not detected.\n";
			return;
		}
		else{
			if (HMD->ProductName[0] == '\0'){
				cout << "Rift detected, display not enabled.\n";
			}
			else{
				//cout << HMD->Handle;
			}
		}






		// the below is setup code for getting the correct distorton mesh
		// it still needs to move to its correct function for the ANE to have access to it.

		#ifndef max
		#define max(a,b)            (((a) > (b)) ? (a) : (b))
		#endif

		//Configure Stereo settings.
		Sizei recommenedTex0Size = ovrHmd_GetFovTextureSize(HMD, ovrEye_Left, HMD->DefaultEyeFov[0], 1.0f);
		Sizei recommenedTex1Size = ovrHmd_GetFovTextureSize(HMD, ovrEye_Right, HMD->DefaultEyeFov[1], 1.0f);
		
		renderTargetSize.w = recommenedTex0Size.w + recommenedTex1Size.w;
		renderTargetSize.h = max(recommenedTex0Size.h, recommenedTex1Size.h);

		// The viewport sizes are re-computed in case RenderTargetSize changed due to HW limitations.
		eyeFov[0] = HMD->DefaultEyeFov[0];
		eyeFov[1] = HMD->DefaultEyeFov[1];

		eyeRenderDesc[0] = ovrHmd_GetRenderDesc(HMD, ovrEye_Left, eyeFov[0]);
		eyeRenderDesc[1] = ovrHmd_GetRenderDesc(HMD, ovrEye_Right, eyeFov[1]);

		//Generate distortion mesh for each eye
		for (int eyeNum = 0; eyeNum < 2; eyeNum++)
		{
			ovrDistortionMesh meshData;

			// Allocate  &  generate  distortion  mesh  vertices. ovrDistortionMesh meshData; 
			ovrHmd_CreateDistortionMesh(HMD, eyeRenderDesc[eyeNum].Eye, eyeRenderDesc[eyeNum].Fov, ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp, &meshData);

			ovrHmd_GetRenderScaleAndOffset(eyeRenderDesc[eyeNum].Fov, renderTargetSize, eyeRenderViewport[eyeNum], UVScaleOffset[eyeNum]);

			

			// Now parse the vertex data and create a render ready vertex buffer from it
			//DistortionVertex * pVBVerts = (DistortionVertex*)OVR_ALLOC(sizeof(DistortionVertex) * meshData.VertexCount);
			//DistortionVertex * v = pVBVerts;
			ovrDistortionVertex * ov = meshData.pVertexData;
			for (unsigned vertNum = 0; vertNum < meshData.VertexCount; vertNum++)
			{
				cout << "\n";
				//v->Pos.x = ov->ScreenPosNDC.x;
				//cout << ov->ScreenPosNDC.x;
				//v->Pos.y = ov->ScreenPosNDC.y;
				//v->TexR = (*(Vector2f*)&ov->TanEyeAnglesR); 
				//v->TexG = (*(Vector2f*)&ov->TanEyeAnglesG); 
				//v->TexB = (*(Vector2f*)&ov->TanEyeAnglesB);
				//v->Col.R = v->Col.G = v->Col.B = (OVR::UByte)(ov->VignetteFactor * 255.99f);
				//v->Col.A = (OVR::UByte)(ov->TimeWarpFactor * 255.99f);
				//v++; 
				ov++;
			}

			//Register this mesh with the renderer DistortionData.MeshVBs[eyeNum]  =  *pRender->CreateBuffer(); 
			//DistortionData.MeshVBs[eyeNum]->Data(Buffer_Vertex, pVBVerts, sizeof(DistortionVertex) * meshData.VertexCount);

			//DistortionData.MeshIBs[eyeNum] = *pRender->CreateBuffer();
			//DistortionData.MeshIBs[eyeNum]->Data(Buffer_Index, meshData.pIndexData, sizeof(unsigned short) * meshData.IndexCount);

			//OVR_FREE(pVBVerts);
			ovrHmd_DestroyDistortionMesh(&meshData);
		}






		ovrHmd_SetEnabledCaps(HMD, ovrHmdCap_DynamicPrediction);

		// Start the sensor which informs of the Rift's pose and motion
		bool result = ovrHmd_ConfigureTracking(HMD, ovrTrackingCap_Orientation |
			ovrTrackingCap_MagYawCorrection |
			ovrTrackingCap_Position, 0);

		//NSLog(@"Tracking passed ? %d", result);
	}

	void OculusANE_ContextFinalizer(FREContext ctx)
	{
		return;
	}

	void OculusANEInitializer(void** extDataToSet, FREContextInitializer* ctxInitializerToSet, FREContextFinalizer* ctxFinalizerToSet){
		extDataToSet = NULL;
		*ctxInitializerToSet = &OculusANE_ContextInitializer;
		*ctxFinalizerToSet = &OculusANE_ContextFinalizer;

		//redirectConsoleLogToDocumentFolder();
	}

	void OculusANEFinalizer(FREContext ctx) {
		if (HMD){
			ovrHmd_Destroy(HMD);
		}

		ovr_Shutdown();
		HMD = NULL;
	}
}