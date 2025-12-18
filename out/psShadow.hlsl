#pragma pack_matrix(row_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 90 "core"
Texture2D<float4 > entryPointParams_diffuse_0 : register(t0);


#line 137 "shaders.hlsl"
Texture2D<float >  entryPointParams_shadow_0[int(4)] : register(t1);


#line 890 "hlsl.meta.slang"
SamplerState entryPointParams_sampler_0 : register(s0);


#line 785
SamplerComparisonState entryPointParams_cmp_0 : register(s1);


#line 4313 "core.meta.slang"
cbuffer entryPointParams_light_0 : register(b0)
{
    float3 entryPointParams_light_0;
}

#line 4313
cbuffer entryPointParams_cascade0_0 : register(b1)
{
    float4x4 entryPointParams_cascade0_0;
}

#line 4313
cbuffer entryPointParams_cascade1_0 : register(b2)
{
    float4x4 entryPointParams_cascade1_0;
}

#line 4313
cbuffer entryPointParams_cascade2_0 : register(b3)
{
    float4x4 entryPointParams_cascade2_0;
}

#line 4313
cbuffer entryPointParams_cascade3_0 : register(b4)
{
    float4x4 entryPointParams_cascade3_0;
}

#line 13 "shaders.hlsl"
float4 x2A_0(float4x4 m_0, float4 v_0)
{
    return mul(m_0, v_0);
}


#line 130
bool inVolume_0(float3 ndc_0)
{

#line 130
    bool _S1;

    if((abs(ndc_0.x)) < 1.0f)
    {

#line 132
        _S1 = (abs(ndc_0.y)) < 1.0f;

#line 132
    }
    else
    {

#line 132
        _S1 = false;

#line 132
    }

#line 132
    if(_S1)
    {

#line 132
        _S1 = (ndc_0.z) > 0.0f;

#line 132
    }
    else
    {

#line 132
        _S1 = false;

#line 132
    }

#line 132
    if(_S1)
    {

#line 132
        _S1 = (ndc_0.z) < 1.0f;

#line 132
    }
    else
    {

#line 132
        _S1 = false;

#line 132
    }

#line 132
    return _S1;
}


#line 124
float2 ndc2uv_0(float2 ndc_1)
{
    float2 _S2 = (ndc_1 + 1.0f) / 2.0f;

#line 124
    float2 _S3 = _S2;


    _S3[int(1)] = 1.0f - _S2.y;
    return _S3;
}


#line 137
void psShadow(float4 svp_0 : SV_Position, float3 normal_0 : normal, float2 uv_0 : uv, float3 pos_0 : pos, out float4 target_0 : SV_Target)
{

#line 137
    float2 _S4 = uv_0;

#line 137
    float3 _S5 = pos_0;

#line 145
    float _S6 = dot(normalize(normal_0), entryPointParams_light_0);

#line 145
    float lambert_0 = 0.75f * ((_S6 + 1.0f) / 2.0f) + 0.25f;

#line 145
    float inLight_0;


    if(_S6 > 0.0f)
    {
        float4 _S7 = float4(_S5, 1.0f);

#line 150
        float3 ndc_2 = x2A_0(entryPointParams_cascade0_0, _S7).xyz;
        if(inVolume_0(ndc_2))
        {

#line 151
            inLight_0 = entryPointParams_shadow_0[int(0)].SampleCmp(entryPointParams_cmp_0, ndc2uv_0(ndc_2.xy), ndc_2.z);

#line 151
        }
        else
        {


            float3 ndc_3 = x2A_0(entryPointParams_cascade1_0, _S7).xyz;
            if(inVolume_0(ndc_3))
            {

#line 157
                inLight_0 = entryPointParams_shadow_0[int(1)].SampleCmp(entryPointParams_cmp_0, ndc2uv_0(ndc_3.xy), ndc_3.z);

#line 157
            }
            else
            {

                float3 ndc_4 = x2A_0(entryPointParams_cascade2_0, _S7).xyz;
                if(inVolume_0(ndc_4))
                {

#line 162
                    inLight_0 = entryPointParams_shadow_0[int(2)].SampleCmp(entryPointParams_cmp_0, ndc2uv_0(ndc_4.xy), ndc_4.z);

#line 162
                }
                else
                {

                    float3 ndc_5 = x2A_0(entryPointParams_cascade3_0, _S7).xyz;
                    if(inVolume_0(ndc_5))
                    {

#line 167
                        inLight_0 = entryPointParams_shadow_0[int(3)].SampleCmp(entryPointParams_cmp_0, ndc2uv_0(ndc_5.xy), ndc_5.z);

#line 167
                    }
                    else
                    {

#line 167
                        inLight_0 = 1.0f;

#line 167
                    }

#line 162
                }

#line 157
            }

#line 151
        }

#line 148
    }
    else
    {

#line 148
        inLight_0 = 1.0f;

#line 148
    }

#line 173
    target_0 = lambert_0 * lerp(0.64999997615814209f, 1.0f, inLight_0) * entryPointParams_diffuse_0.Sample(entryPointParams_sampler_0, _S4);
    return;
}

