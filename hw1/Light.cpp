#include "stdafx.h"
#include "Light.h"

CB_LIGHT_DATA PointLight::MakeLightData()
{
	CB_LIGHT_DATA data{};
	{
		data.xmf4Diffuse = m_xmf4Diffuse;
		data.xmf4Ambient = m_xmf4Ambient;
		data.xmf4Specular = m_xmf4Specular;
		data.xmf3Position = m_xmf3Position;
		data.fRange = m_fRange;
		data.xmf3Attenuation.x = m_fAttenuation0;
		data.xmf3Attenuation.y = m_fAttenuation1;
		data.xmf3Attenuation.z = m_fAttenuation2;
	}

	return data;
}

CB_LIGHT_DATA SpotLight::MakeLightData()
{
	CB_LIGHT_DATA data{};
	{
		data.xmf4Diffuse = m_xmf4Diffuse;
		data.xmf4Ambient = m_xmf4Ambient;
		data.xmf4Specular = m_xmf4Specular;
		data.xmf3Position = m_xmf3Position;
		data.xmf3Direction = m_xmf3Direction;
		data.fRange = m_fRange;
		data.fFalloff = m_fFalloff;
		data.xmf3Attenuation.x = m_fAttenuation0;
		data.xmf3Attenuation.y = m_fAttenuation1;
		data.xmf3Attenuation.z = m_fAttenuation2;
		data.fTheta = m_fTheta;
		data.fPhi = m_fPhi;
	}

	return data;
}

CB_LIGHT_DATA DirectionalLight::MakeLightData()
{
	CB_LIGHT_DATA data{};
	{
		data.xmf4Diffuse = m_xmf4Diffuse;
		data.xmf4Ambient = m_xmf4Ambient;
		data.xmf4Specular = m_xmf4Specular;
		data.xmf3Direction = m_xmf3Direction;
	}

	return data;
}
