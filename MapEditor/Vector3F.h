#pragma once

class Vector3F
{
public:
	Vector3F(float x, float y, float z);
	Vector3F() {
		this->Vector3F::Vector3F(0.0f, 0.0f, 0.0f);
	}
	void SetX(float x);
	void SetY(float y);
	void SetZ(float z);
	float GetX();
	float GetY();
	float GetZ();
	float* GetRawData();
private:
	float m_Data[3];
};