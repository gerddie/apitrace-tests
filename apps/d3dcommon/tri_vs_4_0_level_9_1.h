#if 0
//
// Generated by Microsoft (R) D3D Shader Disassembler
//
//
//   fxc /nologo /Qstrip_reflect /T vs_4_0_level_9_1 /E VS /Fh
//    Z:/projects/apitrace/tests/apps/d3dcommon/tri_vs_4_0_level_9_1.h
//    Z:/projects/apitrace/tests/apps/d3dcommon/tri.fx
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// POSITION                 0   xyzw        0     NONE   float   xyzw
// COLOR                    0   xyzw        1     NONE   float   xyzw
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float   xyzw
// COLOR                    0   xyzw        1     NONE   float   xyzw
//
//
// Runtime generated constant mappings:
//
// Target Reg                               Constant Description
// ---------- --------------------------------------------------
// c0                              Vertex Shader position offset
//
//
// Level9 shader bytecode:
//
    vs_2_0
    dcl_texcoord v0
    dcl_texcoord1 v1
    mad oPos.xy, v0.w, c0, v0
    mov oPos.zw, v0
    mov oT0, v1

// approximately 3 instruction slots used
vs_4_0
dcl_input v0.xyzw
dcl_input v1.xyzw
dcl_output_siv o0.xyzw, position
dcl_output o1.xyzw
mov o0.xyzw, v0.xyzw
mov o1.xyzw, v1.xyzw
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_VS[] =
{
     68,  88,  66,  67, 200, 135, 
    140, 149, 126, 224, 232,  55, 
    199, 118,  64,  71, 204, 172, 
     65, 111,   1,   0,   0,   0, 
    192,   1,   0,   0,   4,   0, 
      0,   0,  48,   0,   0,   0, 
    172,   0,   0,   0,  28,   1, 
      0,   0, 108,   1,   0,   0, 
     65, 111, 110,  57, 116,   0, 
      0,   0, 116,   0,   0,   0, 
      0,   2, 254, 255,  76,   0, 
      0,   0,  40,   0,   0,   0, 
      0,   0,  36,   0,   0,   0, 
     36,   0,   0,   0,  36,   0, 
      0,   0,  36,   0,   1,   0, 
     36,   0,   0,   0,   0,   0, 
      0,   2, 254, 255,  31,   0, 
      0,   2,   5,   0,   0, 128, 
      0,   0,  15, 144,  31,   0, 
      0,   2,   5,   0,   1, 128, 
      1,   0,  15, 144,   4,   0, 
      0,   4,   0,   0,   3, 192, 
      0,   0, 255, 144,   0,   0, 
    228, 160,   0,   0, 228, 144, 
      1,   0,   0,   2,   0,   0, 
     12, 192,   0,   0, 228, 144, 
      1,   0,   0,   2,   0,   0, 
     15, 224,   1,   0, 228, 144, 
    255, 255,   0,   0,  83,  72, 
     68,  82, 104,   0,   0,   0, 
     64,   0,   1,   0,  26,   0, 
      0,   0,  95,   0,   0,   3, 
    242,  16,  16,   0,   0,   0, 
      0,   0,  95,   0,   0,   3, 
    242,  16,  16,   0,   1,   0, 
      0,   0, 103,   0,   0,   4, 
    242,  32,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   1,   0,   0,   0, 
     54,   0,   0,   5, 242,  32, 
     16,   0,   0,   0,   0,   0, 
     70,  30,  16,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
    242,  32,  16,   0,   1,   0, 
      0,   0,  70,  30,  16,   0, 
      1,   0,   0,   0,  62,   0, 
      0,   1,  73,  83,  71,  78, 
     72,   0,   0,   0,   2,   0, 
      0,   0,   8,   0,   0,   0, 
     56,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,  15,   0,   0, 
     65,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      0,   0,  15,  15,   0,   0, 
     80,  79,  83,  73,  84,  73, 
     79,  78,   0,  67,  79,  76, 
     79,  82,   0, 171,  79,  83, 
     71,  78,  76,   0,   0,   0, 
      2,   0,   0,   0,   8,   0, 
      0,   0,  56,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   0,  68,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,  15,   0, 
      0,   0,  83,  86,  95,  80, 
     79,  83,  73,  84,  73,  79, 
     78,   0,  67,  79,  76,  79, 
     82,   0, 171, 171
};
