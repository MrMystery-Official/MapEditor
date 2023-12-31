#include "Vector3F.h"

Vector3F::Vector3F(float x, float y, float z)
{
	this->m_Data[0] = x;
	this->m_Data[1] = y;
	this->m_Data[2] = z;
}

void Vector3F::SetX(float x)
{
	this->m_Data[0] = x;
}

void Vector3F::SetY(float y)
{
	this->m_Data[1] = y;
}

void Vector3F::SetZ(float z)
{
	this->m_Data[2] = z;
}

float Vector3F::GetX()
{
	return this->m_Data[0];
}

float Vector3F::GetY()
{
	return this->m_Data[1];
}

float Vector3F::GetZ()
{
	return this->m_Data[2];
}

float* Vector3F::GetRawData()
{
	return this->m_Data;
}