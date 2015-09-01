/*
 * demo.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_DEMO_H_
#define SRC_DEMO_H_

#include <cstdio>
#include <cmath>
#include <cstdarg>

#include "ObjectDetector.h"
#include "CamInput.h"
#include "CamMarkerDetect.h"
#include "CamMonitor.h"
#include "AutoPilot.h"
#include "Config.h"

#define Z_FAR_LIM 50
#define Z_NEAR_LIM 500

#define PID_UI_LIM 1000.0
#define PID_UI_LIM_HALF 500.0

#define PID_P_LIM 10.0
#define PID_I_LIM 1.0
#define PID_D_LIM 50.0
#define PID_Z_LIM 500.0
#define PID_YAW_P_LIM 200.0
#define DT_LIM 10.0

using namespace kai;


int g_key;
bool g_bRun;
bool g_bTracking;

CamStream* g_pCamFront;
ObjectDetector* g_pOD;
AutoPilot* g_pAP;
CamMarkerDetect* g_pMD;

VehicleInterface* g_pVehicle;
cv::Mat g_displayMat;
fVector3 g_targetPosExt;

JSON g_Json;
FileIO g_file;
Config g_config;
string g_serialPort;


struct PID_UI
{
	int m_P;
	int m_I;
	int m_D;
	int m_Z;
};

PID_UI g_rollFar;
PID_UI g_rollNear;
PID_UI g_altFar;
PID_UI g_altNear;
PID_UI g_pitchFar;
PID_UI g_pitchNear;
PID_UI g_yawFar;
PID_UI g_yawNear;
int g_dT;

void onMouse(int event, int x, int y, int flags, void* userdata);
void createConfigWindow(void);
void onTrackbar(int, void*);
void displayInfo(void);
void handleKey(int key);
void printEnvironment(void);


void printEnvironment(void)
{
	printf("Optimized code: %d\n", useOptimized());
	printf("CUDA devices: %d\n", cuda::getCudaEnabledDeviceCount());
	printf("Current CUDA device: %d\n", cuda::getDevice());

	if (ocl::haveOpenCL())
	{
		printf("OpenCL is found\n");
		ocl::setUseOpenCL(true);
		if (ocl::useOpenCL())
		{
			printf("Using OpenCL\n");
		}
	}
	else
	{
		printf("OpenCL not found\n");
	}

}


void handleKey(int key)
{
	switch (key)
	{
	case 'a':
		//Going forward
		if (g_targetPosExt.m_z > Z_FAR_LIM)
		{
			g_targetPosExt.m_z -= 5;
			g_pAP->setTargetPosCV(g_targetPosExt);
		}
		break;
	case 'z':
		//Going backward
		if (g_targetPosExt.m_z < Z_NEAR_LIM)
		{
			g_targetPosExt.m_z += 5;
			g_pAP->setTargetPosCV(g_targetPosExt);
		}
		break;
	case 'c':
		//Config window
		createConfigWindow();
		break;
	case 'm':
		g_pAP->m_hostSystem.m_mode = OPE_MANUAL;
		break;
	case 27:
		g_bRun = false;	//ESC
		break;
	default:
		break;
	}
}

void onMouse(int event, int x, int y, int flags, void* userdata)
{
//	if (x > CAM_WIDTH || y > CAM_HEIGHT)return;

	switch (event)
	{
	case EVENT_LBUTTONDOWN:
		g_bTracking = true;
		g_targetPosExt = g_pAP->getTargetPosCV();
		g_targetPosExt.m_x = x;
		g_targetPosExt.m_y = y;
//		g_targetPosExt.m_z = g_targetPosExt.m_z;

		g_pAP->setTargetPosCV(g_targetPosExt);
		break;
	case EVENT_LBUTTONUP:
		g_bTracking = false;
		break;
	case EVENT_MOUSEMOVE:
		if (!g_bTracking)
			return;
		g_targetPosExt = g_pAP->getTargetPosCV();
		g_targetPosExt.m_x = x;
		g_targetPosExt.m_y = y;
//		g_targetPosExt.m_z = g_targetPosExt.m_z;
		g_pAP->setTargetPosCV(g_targetPosExt);
		break;
	case EVENT_RBUTTONDOWN:
		//Re-locate the ROI for object recognition
		fVector3 v;
		v.m_x = x;
		v.m_y = y;
		v.m_z = 10;
		g_pAP->m_pCamStream[0].m_pCam->m_pMarkerDetect->setObjROI(v);
		break;
	default:
		break;
	}
}

void onTrackbar(int, void*)
{
	//Get current value
	PID_SETTING pid;
/*
	g_pAP->setDelayTime(
			-DT_LIM * (PID_UI_LIM_HALF - (double) g_dT) / PID_UI_LIM_HALF);

	pid.m_P = -PID_P_LIM * (PID_UI_LIM_HALF - (double) g_rollFar.m_P)
			/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM * (PID_UI_LIM_HALF - (double) g_rollFar.m_I)
			/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM * (PID_UI_LIM_HALF - (double) g_rollFar.m_D)
			/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM * (PID_UI_LIM_HALF - (double) g_rollFar.m_Z)
			/ PID_UI_LIM_HALF;
	g_pAP->setRollFarPID(pid);

	pid.m_P = -PID_P_LIM * (PID_UI_LIM_HALF - (double) g_rollNear.m_P)
			/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM * (PID_UI_LIM_HALF - (double) g_rollNear.m_I)
			/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM * (PID_UI_LIM_HALF - (double) g_rollNear.m_D)
			/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM * (PID_UI_LIM_HALF - (double) g_rollNear.m_Z)
			/ PID_UI_LIM_HALF;
	g_pAP->setRollNearPID(pid);

	pid.m_P = -PID_P_LIM * (PID_UI_LIM_HALF - (double) g_altFar.m_P)
			/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM * (PID_UI_LIM_HALF - (double) g_altFar.m_I)
			/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM * (PID_UI_LIM_HALF - (double) g_altFar.m_D)
			/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM * (PID_UI_LIM_HALF - (double) g_altFar.m_Z)
			/ PID_UI_LIM_HALF;
	g_pAP->setAltFarPID(pid);

	pid.m_P = -PID_P_LIM * (PID_UI_LIM_HALF - (double) g_altNear.m_P)
			/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM * (PID_UI_LIM_HALF - (double) g_altNear.m_I)
			/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM * (PID_UI_LIM_HALF - (double) g_altNear.m_D)
			/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM * (PID_UI_LIM_HALF - (double) g_altNear.m_Z)
			/ PID_UI_LIM_HALF;
	g_pAP->setAltNearPID(pid);

	pid.m_P = -PID_P_LIM * (PID_UI_LIM_HALF - (double) g_pitchFar.m_P)
			/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM * (PID_UI_LIM_HALF - (double) g_pitchFar.m_I)
			/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM * (PID_UI_LIM_HALF - (double) g_pitchFar.m_D)
			/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM * (PID_UI_LIM_HALF - (double) g_pitchFar.m_Z)
			/ PID_UI_LIM_HALF;
	g_pAP->setPitchFarPID(pid);

	pid.m_P = -PID_P_LIM * (PID_UI_LIM_HALF - (double) g_pitchNear.m_P)
			/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM * (PID_UI_LIM_HALF - (double) g_pitchNear.m_I)
			/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM * (PID_UI_LIM_HALF - (double) g_pitchNear.m_D)
			/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM * (PID_UI_LIM_HALF - (double) g_pitchNear.m_Z)
			/ PID_UI_LIM_HALF;
	g_pAP->setPitchNearPID(pid);

	pid.m_P = -PID_YAW_P_LIM * (PID_UI_LIM_HALF - (double) g_yawFar.m_P)
			/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM * (PID_UI_LIM_HALF - (double) g_yawFar.m_I)
			/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM * (PID_UI_LIM_HALF - (double) g_yawFar.m_D)
			/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM * (PID_UI_LIM_HALF - (double) g_yawFar.m_Z)
			/ PID_UI_LIM_HALF;
	g_pAP->setYawFarPID(pid);

	pid.m_P = -PID_YAW_P_LIM * (PID_UI_LIM_HALF - (double) g_yawNear.m_P)
			/ PID_UI_LIM_HALF;
	pid.m_I = -PID_I_LIM * (PID_UI_LIM_HALF - (double) g_yawNear.m_I)
			/ PID_UI_LIM_HALF;
	pid.m_D = -PID_D_LIM * (PID_UI_LIM_HALF - (double) g_yawNear.m_D)
			/ PID_UI_LIM_HALF;
	pid.m_Z = -PID_Z_LIM * (PID_UI_LIM_HALF - (double) g_yawNear.m_Z)
			/ PID_UI_LIM_HALF;
	g_pAP->setYawNearPID(pid);
*/
}

void createConfigWindow(void)
{
//	namedWindow (CONFIG_WINDOW);

	//Get current value
	PID_SETTING pid;
/*
	g_dT = (g_pAP->getDelayTime() / DT_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getRollFarPID();
	g_rollFar.m_P = (pid.m_P / PID_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_rollFar.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_rollFar.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_rollFar.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getRollNearPID();
	g_rollNear.m_P = (pid.m_P / PID_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_rollNear.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_rollNear.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_rollNear.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getAltFarPID();
	g_altFar.m_P = (pid.m_P / PID_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_altFar.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_altFar.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_altFar.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getAltNearPID();
	g_altNear.m_P = (pid.m_P / PID_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_altNear.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_altNear.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_altNear.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getPitchFarPID();
	g_pitchFar.m_P = (pid.m_P / PID_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_pitchFar.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_pitchFar.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_pitchFar.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getPitchNearPID();
	g_pitchNear.m_P = (pid.m_P / PID_P_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_pitchNear.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_pitchNear.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_pitchNear.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getYawFarPID();
	g_yawFar.m_P = (pid.m_P / PID_YAW_P_LIM) * PID_UI_LIM_HALF
			+ PID_UI_LIM_HALF;
	g_yawFar.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_yawFar.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_yawFar.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	pid = g_pAP->getYawNearPID();
	g_yawNear.m_P = (pid.m_P / PID_YAW_P_LIM) * PID_UI_LIM_HALF
			+ PID_UI_LIM_HALF;
	g_yawNear.m_I = (pid.m_I / PID_I_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_yawNear.m_D = (pid.m_D / PID_D_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;
	g_yawNear.m_Z = (pid.m_Z / PID_Z_LIM) * PID_UI_LIM_HALF + PID_UI_LIM_HALF;

	//Trackbar for configuration
	createTrackbar("dT", CONFIG_WINDOW, &g_dT, PID_UI_LIM, onTrackbar);

	//Roll
	createTrackbar("rollFar_P", CONFIG_WINDOW, &g_rollFar.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("rollFar_I", CONFIG_WINDOW, &g_rollFar.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("rollFar_D", CONFIG_WINDOW, &g_rollFar.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("rollFar_Z", CONFIG_WINDOW, &g_rollFar.m_Z, PID_UI_LIM,
			onTrackbar);

	createTrackbar("rollNear_P", CONFIG_WINDOW, &g_rollNear.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("rollNear_I", CONFIG_WINDOW, &g_rollNear.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("rollNear_D", CONFIG_WINDOW, &g_rollNear.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("rollNear_Z", CONFIG_WINDOW, &g_rollNear.m_Z, PID_UI_LIM,
			onTrackbar);

	//Alt
	createTrackbar("altFar_P", CONFIG_WINDOW, &g_altFar.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("altFar_I", CONFIG_WINDOW, &g_altFar.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("altFar_D", CONFIG_WINDOW, &g_altFar.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("altFar_Z", CONFIG_WINDOW, &g_altFar.m_Z, PID_UI_LIM,
			onTrackbar);

	createTrackbar("altNear_P", CONFIG_WINDOW, &g_altNear.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("altNear_I", CONFIG_WINDOW, &g_altNear.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("altNear_D", CONFIG_WINDOW, &g_altNear.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("altNear_Z", CONFIG_WINDOW, &g_altNear.m_Z, PID_UI_LIM,
			onTrackbar);

	//Pitch
	createTrackbar("pitchFar_P", CONFIG_WINDOW, &g_pitchFar.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("pitchFar_I", CONFIG_WINDOW, &g_pitchFar.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("pitchFar_D", CONFIG_WINDOW, &g_pitchFar.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("pitchFar_Z", CONFIG_WINDOW, &g_pitchFar.m_Z, PID_UI_LIM,
			onTrackbar);

	createTrackbar("pitchNear_P", CONFIG_WINDOW, &g_pitchNear.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("pitchNear_I", CONFIG_WINDOW, &g_pitchNear.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("pitchNear_D", CONFIG_WINDOW, &g_pitchNear.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("pitchNear_Z", CONFIG_WINDOW, &g_pitchNear.m_Z, PID_UI_LIM,
			onTrackbar);

	//Yaw
	createTrackbar("yawFar_P", CONFIG_WINDOW, &g_yawFar.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("yawFar_I", CONFIG_WINDOW, &g_yawFar.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("yawFar_D", CONFIG_WINDOW, &g_yawFar.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("yawFar_Z", CONFIG_WINDOW, &g_yawFar.m_Z, PID_UI_LIM,
			onTrackbar);

	createTrackbar("yawNear_P", CONFIG_WINDOW, &g_yawNear.m_P, PID_UI_LIM,
			onTrackbar);
	createTrackbar("yawNear_I", CONFIG_WINDOW, &g_yawNear.m_I, PID_UI_LIM,
			onTrackbar);
	createTrackbar("yawNear_D", CONFIG_WINDOW, &g_yawNear.m_D, PID_UI_LIM,
			onTrackbar);
	createTrackbar("yawNear_Z", CONFIG_WINDOW, &g_yawNear.m_Z, PID_UI_LIM,
			onTrackbar);
			*/
}

void displayInfo(void)
{

	char strBuf[512];
	std::string strInfo;
	PID_SETTING pid;
	CONTROL_AXIS pid3;
	fVector3 vPos;
	fVector3 vAtt;
	cv::Rect roi;
	cv::Mat* pExtMat;
	int i;
	int startPosH = 25;
	int startPosV = 125;
	int lineHeight = 20;
	int* pPWM;
	CONTROL_AXIS pControl;
	i = 0;

	if (!g_pMD->getObjPosition(&vPos))
	{
		vPos.m_x = 0;
		vPos.m_y = 0;
		vPos.m_z = 0;
	}
	if (!g_pMD->getObjAttitude(&vAtt))
	{
		vAtt.m_x = 0;
		vAtt.m_y = 0;
		vAtt.m_z = 0;
	}
	g_targetPosExt = g_pAP->getTargetPosCV();

//	g_displayMat = g_pCamFront->m_pMonitor->m_mat;//g_pAP->Scalar(0);
	g_pCamFront->m_pMonitor->m_mat.copyTo(g_displayMat);

	//External Camera Output
//	pExtMat = &g_frontRGB.m_uFrame;
//	roi = cv::Rect(cv::Point(0, 0), pExtMat->size());
//	pExtMat->copyTo(g_displayMat(roi));

	//Vehicle position
	if(vPos.m_z<0)vPos.m_z=0;
	cv::circle(g_displayMat, Point(vPos.m_x, vPos.m_y), vPos.m_z * 0.5,
			Scalar(0, 255, 0), 3);

	//Target position
	if(g_targetPosExt.m_z<0)g_targetPosExt.m_z=0;
	cv::circle(g_displayMat, Point(g_targetPosExt.m_x, g_targetPosExt.m_y),
			g_targetPosExt.m_z * 0.5, Scalar(0, 255, 255), 3);

	//Title
	cv::putText(g_displayMat, "External CAM", Point(startPosH, 25),
			FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 1);

	//Information
	sprintf(strBuf, "Pos: X=%.2f, Y=%.2f, Z=%.2f", vPos.m_x, vPos.m_y,
			vPos.m_z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	sprintf(strBuf, "Att: R=%.2f, P=%.2f, Y=%.2f", vAtt.m_x, vAtt.m_y,
			vAtt.m_z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	sprintf(strBuf, "Target Pos: X=%.2f, Y=%.2f, Z=%.2f, dT=%.2f",
			g_targetPosExt.m_x, g_targetPosExt.m_y, g_targetPosExt.m_z,
			g_pAP->m_dT);//getDelayTime());
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	i++;

	pid3 = g_pAP->m_roll;//getRollPID();
	sprintf(strBuf, "Roll: P=%.2f, I=%.2f, D=%.2f", pid3.m_P, pid3.m_I,
			pid3.m_D);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid3 = g_pAP->m_alt;//getAltPID();
	sprintf(strBuf, "Alt: P=%.2f, I=%.2f, D=%.2f", pid3.m_P, pid3.m_I,
			pid3.m_D);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid3 = g_pAP->m_pitch;//getPitchPID();
	sprintf(strBuf, "Pitch: P=%.2f, I=%.2f, D=%.2f", pid3.m_P, pid3.m_I,
			pid3.m_D);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid3 = g_pAP->m_yaw;//getYawPID();
	sprintf(strBuf, "Yaw: P=%.2f, I=%.2f, D=%.2f", pid3.m_P, pid3.m_I,
			pid3.m_D);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	i++;

	pid = g_pAP->m_rollFar;//getRollFarPID();
	sprintf(strBuf, "Roll_Far: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P,
			pid.m_I, pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->m_rollNear;//getRollNearPID();
	sprintf(strBuf, "Roll_Near: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P,
			pid.m_I, pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->m_altFar;//getAltFarPID();
	sprintf(strBuf, "Alt_Far: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P, pid.m_I,
			pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->m_altNear;//getAltNearPID();
	sprintf(strBuf, "Alt_Near: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P,
			pid.m_I, pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->m_pitchFar;//getPitchFarPID();
	sprintf(strBuf, "Pitch_Far: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P,
			pid.m_I, pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->m_pitchNear;//getPitchNearPID();
	sprintf(strBuf, "Pitch_Near: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P,
			pid.m_I, pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->m_yawFar;//getYawFarPID();
	sprintf(strBuf, "Yaw_Far: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P, pid.m_I,
			pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pid = g_pAP->m_yawNear;//getYawNearPID();
	sprintf(strBuf, "Yaw_Near: P=%.2f, I=%.2f, D=%.2f, Z=%.2f", pid.m_P,
			pid.m_I, pid.m_D, pid.m_Z);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	i = 0;
	startPosH = 600;

	pPWM = g_pAP->getPWMOutput();
	sprintf(strBuf, "PWM: Roll=%d, Pitch=%d, Alt=%d, Yaw=%d", pPWM[0], pPWM[1],
			pPWM[2], pPWM[3]);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pControl = g_pAP->m_roll;//getRollAxis();
	sprintf(strBuf, "ROLL_I_PWM: %.2f", pControl.m_cvErrInteg * pControl.m_I);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pControl = g_pAP->m_pitch;//getPitchAxis();
	sprintf(strBuf, "PITCH_I_PWM: %.2f",
			pControl.m_cvErrInteg * pControl.m_I);
	cv::putText(g_displayMat, String(strBuf),
			Point(startPosH, startPosV + lineHeight * (++i)),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

}

#endif /* SRC_DEMO_H_ */
