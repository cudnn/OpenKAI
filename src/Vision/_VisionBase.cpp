/*
 * CamBase.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_VisionBase.h"

namespace kai
{

_VisionBase::_VisionBase()
{
	m_bOpen = false;
	m_type = unknownStream;
	m_orientation = 0;
	m_width = 1280;
	m_height = 720;
	m_centerH = 640;
	m_centerV = 360;
	m_bGimbal = false;
	m_isoScale = 1.0;
	m_rotTime = 0;
	m_rotPrev = 0;
	m_angleH = 60;
	m_angleV = 60;
	m_bFlip = false;

	m_pBGR = NULL;
	m_pHSV = NULL;
	m_pGray = NULL;
	m_pDepth = NULL;
}

_VisionBase::~_VisionBase()
{
	DEL(m_pBGR);
	DEL(m_pHSV);
	DEL(m_pGray);
	DEL(m_pDepth);
}

bool _VisionBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("width", &m_width));
	F_INFO(pK->v("height", &m_height));
	F_INFO(pK->v("angleV", &m_angleV));
	F_INFO(pK->v("angleH", &m_angleH));
	F_INFO(pK->v("bGimbal", &m_bGimbal));
	F_INFO(pK->v("isoScale", &m_isoScale));
	F_INFO(pK->v("orientation", (int*)&m_orientation));
	F_INFO(pK->v("bFlip", &m_bFlip));

	m_pBGR = new Frame();

	bool bParam = false;
	F_INFO(pK->v("bGray", &bParam));
	if (bParam)
		m_pGray = new Frame();

	bParam = false;
	F_INFO(pK->v("bHSV", &bParam));
	if (bParam)
		m_pHSV = new Frame();

	m_bOpen = false;
	return true;
}

Frame* _VisionBase::bgr(void)
{
	return m_pBGR;
}

Frame* _VisionBase::hsv(void)
{
	return m_pHSV;
}

Frame* _VisionBase::gray(void)
{
	return m_pGray;
}

Frame* _VisionBase::depth(void)
{
	return m_pDepth;
}

void _VisionBase::getRange(double* pMin, double* pMax)
{
}

uint8_t _VisionBase::getOrientation(void)
{
	return m_orientation;
}

void _VisionBase::info(vInt2* pSize, vInt2* pCenter, vInt2* pAngle)
{
	if(pSize)
	{
		pSize->x = m_width;
		pSize->y = m_height;
	}

	if(pCenter)
	{
		pCenter->x = m_centerH;
		pCenter->y = m_centerV;
	}

	if(pAngle)
	{
		pAngle->x = m_angleH;
		pAngle->y = m_angleV;
	}
}

void _VisionBase::setAttitude(double rollRad, double pitchRad, uint64_t timestamp)
{
	Point2f center(m_centerH, m_centerV);
	double deg = -rollRad * 180.0 * OneOvPI;

	m_rotRoll = getRotationMatrix2D(center, deg, m_isoScale);
	//TODO: add rot estimation

}

VISION_TYPE _VisionBase::getType(void)
{
	return m_type;
}

bool _VisionBase::isOpened(void)
{
	return m_bOpen;
}


}
