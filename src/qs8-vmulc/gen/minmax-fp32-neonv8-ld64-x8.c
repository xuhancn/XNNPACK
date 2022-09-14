// Auto-generated file. Do not edit!
//   Template: src/qs8-vmulc/neon.c.in
//   Generator: tools/xngen
//
// Copyright 2021 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <assert.h>

#include <arm_neon.h>

#include <xnnpack/intrinsics-polyfill.h>
#include <xnnpack/vmul.h>


void xnn_qs8_vmulc_minmax_fp32_ukernel__neonv8_ld64_x8(
    size_t batch,
    const int8_t* input_a,
    const int8_t* input_b,
    int8_t* output,
    const union xnn_qs8_mul_minmax_params params[restrict XNN_MIN_ELEMENTS(1)]) XNN_OOB_READS
{
  const int8x8_t va_zero_point = vld1_dup_s8(params->fp32_neonv8.a_zero_point);
  const float32x4_t vscale = vld1q_dup_f32(&params->fp32_neonv8.scale);
  const int16x8_t voutput_zero_point = vld1q_dup_s16(&params->fp32_neonv8.output_zero_point);
  const int8x8_t voutput_min = vld1_dup_s8(&params->fp32_neonv8.output_min);
  const int8x8_t voutput_max = vld1_dup_s8(&params->fp32_neonv8.output_max);

  const int8x8_t vb = vld1_dup_s8(input_b);
  const int8x8_t vb_zero_point = vld1_dup_s8(params->fp32_neonv8.b_zero_point);
  const int16x8_t vxb = vsubl_s8(vb, vb_zero_point);
  for (; batch >= 8 * sizeof(int8_t); batch -= 8 * sizeof(int8_t)) {
    const int8x8_t va01234567 = vld1_s8(input_a); input_a += 8;

    const int16x8_t vxa01234567 = vsubl_s8(va01234567, va_zero_point);

    int32x4_t vacc0123 = vmull_s16(vget_low_s16(vxa01234567), vget_low_s16(vxb));
    int32x4_t vacc4567 = vmull_s16(vget_high_s16(vxa01234567), vget_high_s16(vxb));

    float32x4_t vfpacc0123 = vcvtq_f32_s32(vacc0123);
    float32x4_t vfpacc4567 = vcvtq_f32_s32(vacc4567);

    vfpacc0123 = vmulq_f32(vfpacc0123, vscale);
    vfpacc4567 = vmulq_f32(vfpacc4567, vscale);

    vacc0123 = vcvtnq_s32_f32(vfpacc0123);
    vacc4567 = vcvtnq_s32_f32(vfpacc4567);

    #if XNN_ARCH_ARM64
      int16x8_t vacc01234567 = vqmovn_high_s32(vqmovn_s32(vacc0123), vacc4567);
    #else
      int16x8_t vacc01234567 = vcombine_s16(vqmovn_s32(vacc0123), vqmovn_s32(vacc4567));
    #endif

    vacc01234567 = vqaddq_s16(vacc01234567, voutput_zero_point);

    #if XNN_ARCH_ARM64
      int8x8_t vout01234567 = vqmovn_s16(vacc01234567);
    #else
      int8x8_t vout01234567 = vqmovn_s16(vacc01234567);
    #endif

    vout01234567 = vmax_s8(vout01234567, voutput_min);

    vout01234567 = vmin_s8(vout01234567, voutput_max);

    vst1_s8(output, vout01234567); output += 8;
  }
  if XNN_UNLIKELY(batch != 0) {
    {
      const int8x8_t va01234567 = vld1_s8(input_a);

      const int16x8_t vxa01234567 = vsubl_s8(va01234567, va_zero_point);

      int32x4_t vacc0123 = vmull_s16(vget_low_s16(vxa01234567), vget_low_s16(vxb));
      int32x4_t vacc4567 = vmull_s16(vget_high_s16(vxa01234567), vget_high_s16(vxb));

      float32x4_t vfpacc0123 = vcvtq_f32_s32(vacc0123);
      float32x4_t vfpacc4567 = vcvtq_f32_s32(vacc4567);

      vfpacc0123 = vmulq_f32(vfpacc0123, vscale);
      vfpacc4567 = vmulq_f32(vfpacc4567, vscale);

      vacc0123 = vcvtnq_s32_f32(vfpacc0123);
      vacc4567 = vcvtnq_s32_f32(vfpacc4567);

      #if XNN_ARCH_ARM64
        int16x8_t vacc01234567 = vqmovn_high_s32(vqmovn_s32(vacc0123), vacc4567);
      #else
        int16x8_t vacc01234567 = vcombine_s16(vqmovn_s32(vacc0123), vqmovn_s32(vacc4567));
      #endif

      vacc01234567 = vqaddq_s16(vacc01234567, voutput_zero_point);

      int8x8_t vout01234567 = vqmovn_s16(vacc01234567);

      vout01234567 = vmax_s8(vout01234567, voutput_min);
      vout01234567 = vmin_s8(vout01234567, voutput_max);
      if (batch & (4 * sizeof(int8_t))) {
        vst1_lane_u32((void*) output, vreinterpret_u32_s8(vout01234567), 0); output += 4;
        vout01234567 = vext_s8(vout01234567, vout01234567, 4);
      }
      if (batch & (2 * sizeof(int8_t))) {
        vst1_lane_u16((void*) output, vreinterpret_u16_s8(vout01234567), 0); output += 2;
        vout01234567 = vext_s8(vout01234567, vout01234567, 2);
      }
      if (batch & (1 * sizeof(int8_t))) {
        vst1_lane_s8(output, vout01234567, 0);
      }
    }
  }
}
