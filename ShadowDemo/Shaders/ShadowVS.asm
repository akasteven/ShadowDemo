//
// Generated by Microsoft (R) HLSL Shader Compiler 6.3.9600.16384
//
//
// Buffer Definitions: 
//
// cbuffer cbPerObject
// {
//
//   float4x4 lightWVP;                 // Offset:    0 Size:    64
//   bool isInstancing;                 // Offset:   64 Size:     4
//   float3 padding;                    // Offset:   68 Size:    12 [unused]
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim Slot Elements
// ------------------------------ ---------- ------- ----------- ---- --------
// cbPerObject                       cbuffer      NA          NA    3        1
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// POSITION                 0   xyz         0     NONE   float   xyz 
// NORMAL                   0   xyz         1     NONE   float       
// WORLD                    0   xyzw        2     NONE   float   xyz 
// WORLD                    1   xyzw        3     NONE   float   xyz 
// WORLD                    2   xyzw        4     NONE   float   xyz 
// WORLD                    3   xyzw        5     NONE   float   xyz 
// SV_InstanceID            0   x           6   INSTID    uint       
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float   xyzw
//
vs_5_0
dcl_globalFlags refactoringAllowed | skipOptimization
dcl_constantbuffer cb3[5], immediateIndexed
dcl_input v0.xyz
dcl_input v2.xyz
dcl_input v3.xyz
dcl_input v4.xyz
dcl_input v5.xyz
dcl_output_siv o0.xyzw, position
dcl_temps 3
//
// Initial variable locations:
//   v0.x <- input.PosL.x; v0.y <- input.PosL.y; v0.z <- input.PosL.z; 
//   v1.x <- input.NorL.x; v1.y <- input.NorL.y; v1.z <- input.NorL.z; 
//   v2.x <- input.World._m00; v2.y <- input.World._m01; v2.z <- input.World._m02; v2.w <- input.World._m03; 
//   v3.x <- input.World._m10; v3.y <- input.World._m11; v3.z <- input.World._m12; v3.w <- input.World._m13; 
//   v4.x <- input.World._m20; v4.y <- input.World._m21; v4.z <- input.World._m22; v4.w <- input.World._m23; 
//   v5.x <- input.World._m30; v5.y <- input.World._m31; v5.z <- input.World._m32; v5.w <- input.World._m33; 
//   v6.x <- input.InstanceId; 
//   o0.x <- <VS return value>.PosH.x; o0.y <- <VS return value>.PosH.y; o0.z <- <VS return value>.PosH.z; o0.w <- <VS return value>.PosH.w
//
#line 33 "D:\Projects\Demo\ShadowDemo\ShadowDemo\Shaders\ShadowMap.hlsl"
mov r0.xyz, v0.xyzx  // r0.x <- posL.x; r0.y <- posL.y; r0.z <- posL.z
ine r1.x, l(0, 0, 0, 0), cb3[4].x
if_nz r1.x
  mul r1.xyz, v0.xxxx, v2.xyzx
  mul r2.xyz, v0.yyyy, v3.xyzx
  add r1.xyz, r1.xyzx, r2.xyzx
  mul r2.xyz, v0.zzzz, v4.xyzx
  add r1.xyz, r1.xyzx, r2.xyzx
  mul r2.xyz, v5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
  add r0.xyz, r1.xyzx, r2.xyzx
endif 
mov r0.w, l(1.000000)
dp4 r1.x, r0.xyzw, cb3[0].xyzw  // r1.x <- output.PosH.x
dp4 r1.y, r0.xyzw, cb3[1].xyzw  // r1.y <- output.PosH.y
dp4 r1.z, r0.xyzw, cb3[2].xyzw  // r1.z <- output.PosH.z
dp4 r1.w, r0.xyzw, cb3[3].xyzw  // r1.w <- output.PosH.w
mov o0.xyzw, r1.xyzw
ret 
// Approximately 18 instruction slots used
