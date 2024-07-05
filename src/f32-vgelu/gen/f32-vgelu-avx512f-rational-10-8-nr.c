// Auto-generated file. Do not edit!
//   Template: src/f32-vgelu/rational-10-8.c.in
//   Generator: tools/xngen
//
// Copyright 2024 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <assert.h>
#include <stddef.h>

#include "xnnpack/simd/f32-avx512f.h"

#include "xnnpack/common.h"
#include "xnnpack/microparams.h"
#include "xnnpack/vunary.h"


void xnn_f32_vgelu_ukernel__avx512f_rational_10_8_nr_u16(
    size_t batch,
    const float* input,
    float* output,
    const union xnn_f32_default_params unused_params[restrict XNN_MIN_ELEMENTS(1)])
{
  assert(batch != 0);
  assert(batch % sizeof(float) == 0);
  assert(input != NULL);
  assert(output != NULL);
  assert(xnn_simd_size_f32 == 16);

  // Cap the inputs to this value as `erf(x/sqrt(2))` will always be `+/-1.0f`
  // beyond this point. This value is chosen as the first floating point
  // number as of which the interpolation returns +/-1.0f.
  #if XNN_SIMD_HAS_NATIVE_FMA || (XNN_ARCH_RISCV && XNN_ENABLE_RISCV_VECTOR)
    XNN_SIMD_CONST_F32(vmax_abs_x, 5.1196560860e+00f);
  #else
    XNN_SIMD_CONST_F32(vmax_abs_x, 5.1216077805e+00);
  #endif  // XNN_SIMD_HAS_NATIVE_FMA

  // The monomial coefficients of the numerator polynomial (odd).
  XNN_SIMD_CONST_F32(valpha_1, 7.978940606117e-01f);
  XNN_SIMD_CONST_F32(valpha_3, 4.256680235267e-02f);
  XNN_SIMD_CONST_F32(valpha_5, 7.090541999787e-03f);
  XNN_SIMD_CONST_F32(valpha_7, -4.493505912251e-05f);
  XNN_SIMD_CONST_F32(valpha_9, -1.184946427202e-06f);

  // The monomial coefficients of the denominator polynomial (even).
  // XNN_SIMD_CONST_F32(vbeta_0, 1.0f);
  XNN_SIMD_CONST_F32(vbeta_2, 2.200737744570e-01f);
  XNN_SIMD_CONST_F32(vbeta_4, 2.048633992672e-02f);
  XNN_SIMD_CONST_F32(vbeta_6, 8.826502016746e-04f);
  XNN_SIMD_CONST_F32(vbeta_8, -1.908174999699e-05f);

  XNN_SIMD_CONST_F32(vone, 1.0f);
  XNN_SIMD_CONST_F32(vhalf, 0.5f);
  // Constant needed for the Newton-Raphson iteration of the reciprocal.
  XNN_SIMD_CONST_F32(vtwo, 2.0f);

  for (; batch >= xnn_simd_bytes_f32; batch -= xnn_simd_bytes_f32) {
    const xnn_simd_f32_t vx_orig = xnn_loadu_f32(input);
    input += xnn_simd_size_f32;

    // Clamp the inputs to the interpolation range.
    xnn_simd_f32_t vx = xnn_min_f32(vmax_abs_x, vx_orig);
    vx = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx);

    // Since the polynomials are odd/even, we need x^2.
    const xnn_simd_f32_t vx2 = xnn_mul_f32(vx, vx);

    // Evaluate the numerator polynomial p.
    xnn_simd_f32_t vp = xnn_fmadd_f32(vx2, valpha_9, valpha_7);
    vp = xnn_fmadd_f32(vx2, vp, valpha_5);
    vp = xnn_fmadd_f32(vx2, vp, valpha_3);
    vp = xnn_fmadd_f32(vx2, vp, valpha_1);
    vp = xnn_mul_f32(vx, vp);

    // Evaluate the denominator polynomial q.
    xnn_simd_f32_t vq = xnn_fmadd_f32(vx2, vbeta_8, vbeta_6);
    vq = xnn_fmadd_f32(vx2, vq, vbeta_4);
    vq = xnn_fmadd_f32(vx2, vq, vbeta_2);
    vq = xnn_fmadd_f32(vx2, vq, vone);

    // Divide the numerator by the denominator and add one
    xnn_simd_f32_t vrq = xnn_rcp_f32(vq);
    for (size_t iter = 0; iter < XNN_SIMD_NUM_RCP_ITER_F32; iter++) {
      vrq = xnn_mul_f32(vrq, xnn_fnmadd_f32(vrq, vq, vtwo));
    }
    // Note that we _could_ use a fused multiply-add to compute `p * rq + 1`,
    // but we actually want this to round to zero near the edges, so we
    // don't want the extended precision of the fused multiply-add.
    const xnn_simd_f32_t verf = xnn_mul_f32(vp, vrq);

    // Add one to the rational interpolant, and multiply by 0.5 times the
    // original input.
    const xnn_simd_f32_t vy = xnn_mul_f32(xnn_mul_f32(vx_orig, vhalf),
                                          xnn_add_f32(verf, vone));

    xnn_storeu_f32(output, vy);
    output += xnn_simd_size_f32;
  }
  if XNN_UNLIKELY(batch != 0) {
    xnn_simd_f32_t vx_orig = xnn_load_tail_f32(input, batch >> XNN_LOG2_SIZEOF_FLOAT);

  // See above for comments.
  xnn_simd_f32_t vx = xnn_min_f32(vmax_abs_x, vx_orig);
  vx = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx);
  const xnn_simd_f32_t vx2 = xnn_mul_f32(vx, vx);
  xnn_simd_f32_t vp = xnn_fmadd_f32(vx2, valpha_9, valpha_7);
  vp = xnn_fmadd_f32(vx2, vp, valpha_5);
  vp = xnn_fmadd_f32(vx2, vp, valpha_3);
  vp = xnn_fmadd_f32(vx2, vp, valpha_1);
  vp = xnn_mul_f32(vx, vp);
  xnn_simd_f32_t vq = xnn_fmadd_f32(vx2, vbeta_8, vbeta_6);
  vq = xnn_fmadd_f32(vx2, vq, vbeta_4);
  vq = xnn_fmadd_f32(vx2, vq, vbeta_2);
  vq = xnn_fmadd_f32(vx2, vq, vone);
  xnn_simd_f32_t vrq = xnn_rcp_f32(vq);
  for (size_t iter = 0; iter < XNN_SIMD_NUM_RCP_ITER_F32; iter++) {
    vrq = xnn_mul_f32(vrq, xnn_fnmadd_f32(vrq, vq, vtwo));
  }
  const xnn_simd_f32_t verf = xnn_mul_f32(vp, vrq);
  const xnn_simd_f32_t vy = xnn_mul_f32(xnn_mul_f32(vx_orig, vhalf),
                                        xnn_add_f32(verf, vone));

    xnn_store_tail_f32(output, vy, batch >> XNN_LOG2_SIZEOF_FLOAT);
  }
}

void xnn_f32_vgelu_ukernel__avx512f_rational_10_8_nr_u32(
    size_t batch,
    const float* input,
    float* output,
    const union xnn_f32_default_params unused_params[restrict XNN_MIN_ELEMENTS(1)])
{
  assert(batch != 0);
  assert(batch % sizeof(float) == 0);
  assert(input != NULL);
  assert(output != NULL);
  assert(xnn_simd_size_f32 == 16);

  // Cap the inputs to this value as `erf(x/sqrt(2))` will always be `+/-1.0f`
  // beyond this point. This value is chosen as the first floating point
  // number as of which the interpolation returns +/-1.0f.
  #if XNN_SIMD_HAS_NATIVE_FMA || (XNN_ARCH_RISCV && XNN_ENABLE_RISCV_VECTOR)
    XNN_SIMD_CONST_F32(vmax_abs_x, 5.1196560860e+00f);
  #else
    XNN_SIMD_CONST_F32(vmax_abs_x, 5.1216077805e+00);
  #endif  // XNN_SIMD_HAS_NATIVE_FMA

  // The monomial coefficients of the numerator polynomial (odd).
  XNN_SIMD_CONST_F32(valpha_1, 7.978940606117e-01f);
  XNN_SIMD_CONST_F32(valpha_3, 4.256680235267e-02f);
  XNN_SIMD_CONST_F32(valpha_5, 7.090541999787e-03f);
  XNN_SIMD_CONST_F32(valpha_7, -4.493505912251e-05f);
  XNN_SIMD_CONST_F32(valpha_9, -1.184946427202e-06f);

  // The monomial coefficients of the denominator polynomial (even).
  // XNN_SIMD_CONST_F32(vbeta_0, 1.0f);
  XNN_SIMD_CONST_F32(vbeta_2, 2.200737744570e-01f);
  XNN_SIMD_CONST_F32(vbeta_4, 2.048633992672e-02f);
  XNN_SIMD_CONST_F32(vbeta_6, 8.826502016746e-04f);
  XNN_SIMD_CONST_F32(vbeta_8, -1.908174999699e-05f);

  XNN_SIMD_CONST_F32(vone, 1.0f);
  XNN_SIMD_CONST_F32(vhalf, 0.5f);
  // Constant needed for the Newton-Raphson iteration of the reciprocal.
  XNN_SIMD_CONST_F32(vtwo, 2.0f);

  for (; batch >= 32 * sizeof(float); batch -= 32 * sizeof(float)) {
    const xnn_simd_f32_t vx_orig_0 = xnn_loadu_f32(input);
    const xnn_simd_f32_t vx_orig_1 = xnn_loadu_f32(input + 1 * xnn_simd_size_f32);
    input += 32;

    // Clamp the inputs to the interpolation range.
    xnn_simd_f32_t vx_0 = xnn_min_f32(vmax_abs_x, vx_orig_0);
    xnn_simd_f32_t vx_1 = xnn_min_f32(vmax_abs_x, vx_orig_1);
    vx_0 = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx_0);
    vx_1 = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx_1);

    // Since the polynomials are odd/even, we need x^2.
    const xnn_simd_f32_t vx2_0 = xnn_mul_f32(vx_0, vx_0);
    const xnn_simd_f32_t vx2_1 = xnn_mul_f32(vx_1, vx_1);

    // Evaluate the numerator polynomial p.
    xnn_simd_f32_t vp_0 = xnn_fmadd_f32(vx2_0, valpha_9, valpha_7);
    xnn_simd_f32_t vp_1 = xnn_fmadd_f32(vx2_1, valpha_9, valpha_7);
    vp_0 = xnn_fmadd_f32(vx2_0, vp_0, valpha_5);
    vp_1 = xnn_fmadd_f32(vx2_1, vp_1, valpha_5);
    vp_0 = xnn_fmadd_f32(vx2_0, vp_0, valpha_3);
    vp_1 = xnn_fmadd_f32(vx2_1, vp_1, valpha_3);
    vp_0 = xnn_fmadd_f32(vx2_0, vp_0, valpha_1);
    vp_1 = xnn_fmadd_f32(vx2_1, vp_1, valpha_1);
    vp_0 = xnn_mul_f32(vx_0, vp_0);
    vp_1 = xnn_mul_f32(vx_1, vp_1);

    // Evaluate the denominator polynomial q.
    xnn_simd_f32_t vq_0 = xnn_fmadd_f32(vx2_0, vbeta_8, vbeta_6);
    xnn_simd_f32_t vq_1 = xnn_fmadd_f32(vx2_1, vbeta_8, vbeta_6);
    vq_0 = xnn_fmadd_f32(vx2_0, vq_0, vbeta_4);
    vq_1 = xnn_fmadd_f32(vx2_1, vq_1, vbeta_4);
    vq_0 = xnn_fmadd_f32(vx2_0, vq_0, vbeta_2);
    vq_1 = xnn_fmadd_f32(vx2_1, vq_1, vbeta_2);
    vq_0 = xnn_fmadd_f32(vx2_0, vq_0, vone);
    vq_1 = xnn_fmadd_f32(vx2_1, vq_1, vone);

    // Divide the numerator by the denominator.
    xnn_simd_f32_t vrq_0 = xnn_rcp_f32(vq_0);
    xnn_simd_f32_t vrq_1 = xnn_rcp_f32(vq_1);
    for (size_t iter = 0; iter < XNN_SIMD_NUM_RCP_ITER_F32; iter++) {
      vrq_0 = xnn_mul_f32(vrq_0, xnn_fnmadd_f32(vrq_0, vq_0, vtwo));
      vrq_1 = xnn_mul_f32(vrq_1, xnn_fnmadd_f32(vrq_1, vq_1, vtwo));
    }
    // Note that we _could_ use a fused multiply-add to compute `p * rq + 1`,
    // but we actually want this to round to zero near the edges, so we
    // don't want the extended precision of the fused multiply-add.
    const xnn_simd_f32_t verf_0 = xnn_mul_f32(vp_0, vrq_0);
    const xnn_simd_f32_t verf_1 = xnn_mul_f32(vp_1, vrq_1);

    // Add one to the rational interpolant, and multiply by 0.5 times the
    // original input.
    const xnn_simd_f32_t vy_0 = xnn_mul_f32(xnn_mul_f32(vx_orig_0, vhalf),
                                        xnn_add_f32(verf_0, vone));
    const xnn_simd_f32_t vy_1 = xnn_mul_f32(xnn_mul_f32(vx_orig_1, vhalf),
                                        xnn_add_f32(verf_1, vone));

    xnn_storeu_f32(output, vy_0);
    xnn_storeu_f32(output + 1 * xnn_simd_size_f32, vy_1);
    output += 32;
  }
  for (; batch >= xnn_simd_bytes_f32; batch -= xnn_simd_bytes_f32) {
    const xnn_simd_f32_t vx_orig = xnn_loadu_f32(input);
    input += xnn_simd_size_f32;

    // Clamp the inputs to the interpolation range.
    xnn_simd_f32_t vx = xnn_min_f32(vmax_abs_x, vx_orig);
    vx = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx);

    // Since the polynomials are odd/even, we need x^2.
    const xnn_simd_f32_t vx2 = xnn_mul_f32(vx, vx);

    // Evaluate the numerator polynomial p.
    xnn_simd_f32_t vp = xnn_fmadd_f32(vx2, valpha_9, valpha_7);
    vp = xnn_fmadd_f32(vx2, vp, valpha_5);
    vp = xnn_fmadd_f32(vx2, vp, valpha_3);
    vp = xnn_fmadd_f32(vx2, vp, valpha_1);
    vp = xnn_mul_f32(vx, vp);

    // Evaluate the denominator polynomial q.
    xnn_simd_f32_t vq = xnn_fmadd_f32(vx2, vbeta_8, vbeta_6);
    vq = xnn_fmadd_f32(vx2, vq, vbeta_4);
    vq = xnn_fmadd_f32(vx2, vq, vbeta_2);
    vq = xnn_fmadd_f32(vx2, vq, vone);

    // Divide the numerator by the denominator and add one
    xnn_simd_f32_t vrq = xnn_rcp_f32(vq);
    for (size_t iter = 0; iter < XNN_SIMD_NUM_RCP_ITER_F32; iter++) {
      vrq = xnn_mul_f32(vrq, xnn_fnmadd_f32(vrq, vq, vtwo));
    }
    // Note that we _could_ use a fused multiply-add to compute `p * rq + 1`,
    // but we actually want this to round to zero near the edges, so we
    // don't want the extended precision of the fused multiply-add.
    const xnn_simd_f32_t verf = xnn_mul_f32(vp, vrq);

    // Add one to the rational interpolant, and multiply by 0.5 times the
    // original input.
    const xnn_simd_f32_t vy = xnn_mul_f32(xnn_mul_f32(vx_orig, vhalf),
                                          xnn_add_f32(verf, vone));

    xnn_storeu_f32(output, vy);
    output += xnn_simd_size_f32;
  }
  if XNN_UNLIKELY(batch != 0) {
    xnn_simd_f32_t vx_orig = xnn_load_tail_f32(input, batch >> XNN_LOG2_SIZEOF_FLOAT);

  // See above for comments.
  xnn_simd_f32_t vx = xnn_min_f32(vmax_abs_x, vx_orig);
  vx = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx);
  const xnn_simd_f32_t vx2 = xnn_mul_f32(vx, vx);
  xnn_simd_f32_t vp = xnn_fmadd_f32(vx2, valpha_9, valpha_7);
  vp = xnn_fmadd_f32(vx2, vp, valpha_5);
  vp = xnn_fmadd_f32(vx2, vp, valpha_3);
  vp = xnn_fmadd_f32(vx2, vp, valpha_1);
  vp = xnn_mul_f32(vx, vp);
  xnn_simd_f32_t vq = xnn_fmadd_f32(vx2, vbeta_8, vbeta_6);
  vq = xnn_fmadd_f32(vx2, vq, vbeta_4);
  vq = xnn_fmadd_f32(vx2, vq, vbeta_2);
  vq = xnn_fmadd_f32(vx2, vq, vone);
  xnn_simd_f32_t vrq = xnn_rcp_f32(vq);
  for (size_t iter = 0; iter < XNN_SIMD_NUM_RCP_ITER_F32; iter++) {
    vrq = xnn_mul_f32(vrq, xnn_fnmadd_f32(vrq, vq, vtwo));
  }
  const xnn_simd_f32_t verf = xnn_mul_f32(vp, vrq);
  const xnn_simd_f32_t vy = xnn_mul_f32(xnn_mul_f32(vx_orig, vhalf),
                                        xnn_add_f32(verf, vone));

    xnn_store_tail_f32(output, vy, batch >> XNN_LOG2_SIZEOF_FLOAT);
  }
}

void xnn_f32_vgelu_ukernel__avx512f_rational_10_8_nr_u48(
    size_t batch,
    const float* input,
    float* output,
    const union xnn_f32_default_params unused_params[restrict XNN_MIN_ELEMENTS(1)])
{
  assert(batch != 0);
  assert(batch % sizeof(float) == 0);
  assert(input != NULL);
  assert(output != NULL);
  assert(xnn_simd_size_f32 == 16);

  // Cap the inputs to this value as `erf(x/sqrt(2))` will always be `+/-1.0f`
  // beyond this point. This value is chosen as the first floating point
  // number as of which the interpolation returns +/-1.0f.
  #if XNN_SIMD_HAS_NATIVE_FMA || (XNN_ARCH_RISCV && XNN_ENABLE_RISCV_VECTOR)
    XNN_SIMD_CONST_F32(vmax_abs_x, 5.1196560860e+00f);
  #else
    XNN_SIMD_CONST_F32(vmax_abs_x, 5.1216077805e+00);
  #endif  // XNN_SIMD_HAS_NATIVE_FMA

  // The monomial coefficients of the numerator polynomial (odd).
  XNN_SIMD_CONST_F32(valpha_1, 7.978940606117e-01f);
  XNN_SIMD_CONST_F32(valpha_3, 4.256680235267e-02f);
  XNN_SIMD_CONST_F32(valpha_5, 7.090541999787e-03f);
  XNN_SIMD_CONST_F32(valpha_7, -4.493505912251e-05f);
  XNN_SIMD_CONST_F32(valpha_9, -1.184946427202e-06f);

  // The monomial coefficients of the denominator polynomial (even).
  // XNN_SIMD_CONST_F32(vbeta_0, 1.0f);
  XNN_SIMD_CONST_F32(vbeta_2, 2.200737744570e-01f);
  XNN_SIMD_CONST_F32(vbeta_4, 2.048633992672e-02f);
  XNN_SIMD_CONST_F32(vbeta_6, 8.826502016746e-04f);
  XNN_SIMD_CONST_F32(vbeta_8, -1.908174999699e-05f);

  XNN_SIMD_CONST_F32(vone, 1.0f);
  XNN_SIMD_CONST_F32(vhalf, 0.5f);
  // Constant needed for the Newton-Raphson iteration of the reciprocal.
  XNN_SIMD_CONST_F32(vtwo, 2.0f);

  for (; batch >= 48 * sizeof(float); batch -= 48 * sizeof(float)) {
    const xnn_simd_f32_t vx_orig_0 = xnn_loadu_f32(input);
    const xnn_simd_f32_t vx_orig_1 = xnn_loadu_f32(input + 1 * xnn_simd_size_f32);
    const xnn_simd_f32_t vx_orig_2 = xnn_loadu_f32(input + 2 * xnn_simd_size_f32);
    input += 48;

    // Clamp the inputs to the interpolation range.
    xnn_simd_f32_t vx_0 = xnn_min_f32(vmax_abs_x, vx_orig_0);
    xnn_simd_f32_t vx_1 = xnn_min_f32(vmax_abs_x, vx_orig_1);
    xnn_simd_f32_t vx_2 = xnn_min_f32(vmax_abs_x, vx_orig_2);
    vx_0 = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx_0);
    vx_1 = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx_1);
    vx_2 = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx_2);

    // Since the polynomials are odd/even, we need x^2.
    const xnn_simd_f32_t vx2_0 = xnn_mul_f32(vx_0, vx_0);
    const xnn_simd_f32_t vx2_1 = xnn_mul_f32(vx_1, vx_1);
    const xnn_simd_f32_t vx2_2 = xnn_mul_f32(vx_2, vx_2);

    // Evaluate the numerator polynomial p.
    xnn_simd_f32_t vp_0 = xnn_fmadd_f32(vx2_0, valpha_9, valpha_7);
    xnn_simd_f32_t vp_1 = xnn_fmadd_f32(vx2_1, valpha_9, valpha_7);
    xnn_simd_f32_t vp_2 = xnn_fmadd_f32(vx2_2, valpha_9, valpha_7);
    vp_0 = xnn_fmadd_f32(vx2_0, vp_0, valpha_5);
    vp_1 = xnn_fmadd_f32(vx2_1, vp_1, valpha_5);
    vp_2 = xnn_fmadd_f32(vx2_2, vp_2, valpha_5);
    vp_0 = xnn_fmadd_f32(vx2_0, vp_0, valpha_3);
    vp_1 = xnn_fmadd_f32(vx2_1, vp_1, valpha_3);
    vp_2 = xnn_fmadd_f32(vx2_2, vp_2, valpha_3);
    vp_0 = xnn_fmadd_f32(vx2_0, vp_0, valpha_1);
    vp_1 = xnn_fmadd_f32(vx2_1, vp_1, valpha_1);
    vp_2 = xnn_fmadd_f32(vx2_2, vp_2, valpha_1);
    vp_0 = xnn_mul_f32(vx_0, vp_0);
    vp_1 = xnn_mul_f32(vx_1, vp_1);
    vp_2 = xnn_mul_f32(vx_2, vp_2);

    // Evaluate the denominator polynomial q.
    xnn_simd_f32_t vq_0 = xnn_fmadd_f32(vx2_0, vbeta_8, vbeta_6);
    xnn_simd_f32_t vq_1 = xnn_fmadd_f32(vx2_1, vbeta_8, vbeta_6);
    xnn_simd_f32_t vq_2 = xnn_fmadd_f32(vx2_2, vbeta_8, vbeta_6);
    vq_0 = xnn_fmadd_f32(vx2_0, vq_0, vbeta_4);
    vq_1 = xnn_fmadd_f32(vx2_1, vq_1, vbeta_4);
    vq_2 = xnn_fmadd_f32(vx2_2, vq_2, vbeta_4);
    vq_0 = xnn_fmadd_f32(vx2_0, vq_0, vbeta_2);
    vq_1 = xnn_fmadd_f32(vx2_1, vq_1, vbeta_2);
    vq_2 = xnn_fmadd_f32(vx2_2, vq_2, vbeta_2);
    vq_0 = xnn_fmadd_f32(vx2_0, vq_0, vone);
    vq_1 = xnn_fmadd_f32(vx2_1, vq_1, vone);
    vq_2 = xnn_fmadd_f32(vx2_2, vq_2, vone);

    // Divide the numerator by the denominator.
    xnn_simd_f32_t vrq_0 = xnn_rcp_f32(vq_0);
    xnn_simd_f32_t vrq_1 = xnn_rcp_f32(vq_1);
    xnn_simd_f32_t vrq_2 = xnn_rcp_f32(vq_2);
    for (size_t iter = 0; iter < XNN_SIMD_NUM_RCP_ITER_F32; iter++) {
      vrq_0 = xnn_mul_f32(vrq_0, xnn_fnmadd_f32(vrq_0, vq_0, vtwo));
      vrq_1 = xnn_mul_f32(vrq_1, xnn_fnmadd_f32(vrq_1, vq_1, vtwo));
      vrq_2 = xnn_mul_f32(vrq_2, xnn_fnmadd_f32(vrq_2, vq_2, vtwo));
    }
    // Note that we _could_ use a fused multiply-add to compute `p * rq + 1`,
    // but we actually want this to round to zero near the edges, so we
    // don't want the extended precision of the fused multiply-add.
    const xnn_simd_f32_t verf_0 = xnn_mul_f32(vp_0, vrq_0);
    const xnn_simd_f32_t verf_1 = xnn_mul_f32(vp_1, vrq_1);
    const xnn_simd_f32_t verf_2 = xnn_mul_f32(vp_2, vrq_2);

    // Add one to the rational interpolant, and multiply by 0.5 times the
    // original input.
    const xnn_simd_f32_t vy_0 = xnn_mul_f32(xnn_mul_f32(vx_orig_0, vhalf),
                                        xnn_add_f32(verf_0, vone));
    const xnn_simd_f32_t vy_1 = xnn_mul_f32(xnn_mul_f32(vx_orig_1, vhalf),
                                        xnn_add_f32(verf_1, vone));
    const xnn_simd_f32_t vy_2 = xnn_mul_f32(xnn_mul_f32(vx_orig_2, vhalf),
                                        xnn_add_f32(verf_2, vone));

    xnn_storeu_f32(output, vy_0);
    xnn_storeu_f32(output + 1 * xnn_simd_size_f32, vy_1);
    xnn_storeu_f32(output + 2 * xnn_simd_size_f32, vy_2);
    output += 48;
  }
  for (; batch >= xnn_simd_bytes_f32; batch -= xnn_simd_bytes_f32) {
    const xnn_simd_f32_t vx_orig = xnn_loadu_f32(input);
    input += xnn_simd_size_f32;

    // Clamp the inputs to the interpolation range.
    xnn_simd_f32_t vx = xnn_min_f32(vmax_abs_x, vx_orig);
    vx = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx);

    // Since the polynomials are odd/even, we need x^2.
    const xnn_simd_f32_t vx2 = xnn_mul_f32(vx, vx);

    // Evaluate the numerator polynomial p.
    xnn_simd_f32_t vp = xnn_fmadd_f32(vx2, valpha_9, valpha_7);
    vp = xnn_fmadd_f32(vx2, vp, valpha_5);
    vp = xnn_fmadd_f32(vx2, vp, valpha_3);
    vp = xnn_fmadd_f32(vx2, vp, valpha_1);
    vp = xnn_mul_f32(vx, vp);

    // Evaluate the denominator polynomial q.
    xnn_simd_f32_t vq = xnn_fmadd_f32(vx2, vbeta_8, vbeta_6);
    vq = xnn_fmadd_f32(vx2, vq, vbeta_4);
    vq = xnn_fmadd_f32(vx2, vq, vbeta_2);
    vq = xnn_fmadd_f32(vx2, vq, vone);

    // Divide the numerator by the denominator and add one
    xnn_simd_f32_t vrq = xnn_rcp_f32(vq);
    for (size_t iter = 0; iter < XNN_SIMD_NUM_RCP_ITER_F32; iter++) {
      vrq = xnn_mul_f32(vrq, xnn_fnmadd_f32(vrq, vq, vtwo));
    }
    // Note that we _could_ use a fused multiply-add to compute `p * rq + 1`,
    // but we actually want this to round to zero near the edges, so we
    // don't want the extended precision of the fused multiply-add.
    const xnn_simd_f32_t verf = xnn_mul_f32(vp, vrq);

    // Add one to the rational interpolant, and multiply by 0.5 times the
    // original input.
    const xnn_simd_f32_t vy = xnn_mul_f32(xnn_mul_f32(vx_orig, vhalf),
                                          xnn_add_f32(verf, vone));

    xnn_storeu_f32(output, vy);
    output += xnn_simd_size_f32;
  }
  if XNN_UNLIKELY(batch != 0) {
    xnn_simd_f32_t vx_orig = xnn_load_tail_f32(input, batch >> XNN_LOG2_SIZEOF_FLOAT);

  // See above for comments.
  xnn_simd_f32_t vx = xnn_min_f32(vmax_abs_x, vx_orig);
  vx = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx);
  const xnn_simd_f32_t vx2 = xnn_mul_f32(vx, vx);
  xnn_simd_f32_t vp = xnn_fmadd_f32(vx2, valpha_9, valpha_7);
  vp = xnn_fmadd_f32(vx2, vp, valpha_5);
  vp = xnn_fmadd_f32(vx2, vp, valpha_3);
  vp = xnn_fmadd_f32(vx2, vp, valpha_1);
  vp = xnn_mul_f32(vx, vp);
  xnn_simd_f32_t vq = xnn_fmadd_f32(vx2, vbeta_8, vbeta_6);
  vq = xnn_fmadd_f32(vx2, vq, vbeta_4);
  vq = xnn_fmadd_f32(vx2, vq, vbeta_2);
  vq = xnn_fmadd_f32(vx2, vq, vone);
  xnn_simd_f32_t vrq = xnn_rcp_f32(vq);
  for (size_t iter = 0; iter < XNN_SIMD_NUM_RCP_ITER_F32; iter++) {
    vrq = xnn_mul_f32(vrq, xnn_fnmadd_f32(vrq, vq, vtwo));
  }
  const xnn_simd_f32_t verf = xnn_mul_f32(vp, vrq);
  const xnn_simd_f32_t vy = xnn_mul_f32(xnn_mul_f32(vx_orig, vhalf),
                                        xnn_add_f32(verf, vone));

    xnn_store_tail_f32(output, vy, batch >> XNN_LOG2_SIZEOF_FLOAT);
  }
}

void xnn_f32_vgelu_ukernel__avx512f_rational_10_8_nr_u64(
    size_t batch,
    const float* input,
    float* output,
    const union xnn_f32_default_params unused_params[restrict XNN_MIN_ELEMENTS(1)])
{
  assert(batch != 0);
  assert(batch % sizeof(float) == 0);
  assert(input != NULL);
  assert(output != NULL);
  assert(xnn_simd_size_f32 == 16);

  // Cap the inputs to this value as `erf(x/sqrt(2))` will always be `+/-1.0f`
  // beyond this point. This value is chosen as the first floating point
  // number as of which the interpolation returns +/-1.0f.
  #if XNN_SIMD_HAS_NATIVE_FMA || (XNN_ARCH_RISCV && XNN_ENABLE_RISCV_VECTOR)
    XNN_SIMD_CONST_F32(vmax_abs_x, 5.1196560860e+00f);
  #else
    XNN_SIMD_CONST_F32(vmax_abs_x, 5.1216077805e+00);
  #endif  // XNN_SIMD_HAS_NATIVE_FMA

  // The monomial coefficients of the numerator polynomial (odd).
  XNN_SIMD_CONST_F32(valpha_1, 7.978940606117e-01f);
  XNN_SIMD_CONST_F32(valpha_3, 4.256680235267e-02f);
  XNN_SIMD_CONST_F32(valpha_5, 7.090541999787e-03f);
  XNN_SIMD_CONST_F32(valpha_7, -4.493505912251e-05f);
  XNN_SIMD_CONST_F32(valpha_9, -1.184946427202e-06f);

  // The monomial coefficients of the denominator polynomial (even).
  // XNN_SIMD_CONST_F32(vbeta_0, 1.0f);
  XNN_SIMD_CONST_F32(vbeta_2, 2.200737744570e-01f);
  XNN_SIMD_CONST_F32(vbeta_4, 2.048633992672e-02f);
  XNN_SIMD_CONST_F32(vbeta_6, 8.826502016746e-04f);
  XNN_SIMD_CONST_F32(vbeta_8, -1.908174999699e-05f);

  XNN_SIMD_CONST_F32(vone, 1.0f);
  XNN_SIMD_CONST_F32(vhalf, 0.5f);
  // Constant needed for the Newton-Raphson iteration of the reciprocal.
  XNN_SIMD_CONST_F32(vtwo, 2.0f);

  for (; batch >= 64 * sizeof(float); batch -= 64 * sizeof(float)) {
    const xnn_simd_f32_t vx_orig_0 = xnn_loadu_f32(input);
    const xnn_simd_f32_t vx_orig_1 = xnn_loadu_f32(input + 1 * xnn_simd_size_f32);
    const xnn_simd_f32_t vx_orig_2 = xnn_loadu_f32(input + 2 * xnn_simd_size_f32);
    const xnn_simd_f32_t vx_orig_3 = xnn_loadu_f32(input + 3 * xnn_simd_size_f32);
    input += 64;

    // Clamp the inputs to the interpolation range.
    xnn_simd_f32_t vx_0 = xnn_min_f32(vmax_abs_x, vx_orig_0);
    xnn_simd_f32_t vx_1 = xnn_min_f32(vmax_abs_x, vx_orig_1);
    xnn_simd_f32_t vx_2 = xnn_min_f32(vmax_abs_x, vx_orig_2);
    xnn_simd_f32_t vx_3 = xnn_min_f32(vmax_abs_x, vx_orig_3);
    vx_0 = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx_0);
    vx_1 = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx_1);
    vx_2 = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx_2);
    vx_3 = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx_3);

    // Since the polynomials are odd/even, we need x^2.
    const xnn_simd_f32_t vx2_0 = xnn_mul_f32(vx_0, vx_0);
    const xnn_simd_f32_t vx2_1 = xnn_mul_f32(vx_1, vx_1);
    const xnn_simd_f32_t vx2_2 = xnn_mul_f32(vx_2, vx_2);
    const xnn_simd_f32_t vx2_3 = xnn_mul_f32(vx_3, vx_3);

    // Evaluate the numerator polynomial p.
    xnn_simd_f32_t vp_0 = xnn_fmadd_f32(vx2_0, valpha_9, valpha_7);
    xnn_simd_f32_t vp_1 = xnn_fmadd_f32(vx2_1, valpha_9, valpha_7);
    xnn_simd_f32_t vp_2 = xnn_fmadd_f32(vx2_2, valpha_9, valpha_7);
    xnn_simd_f32_t vp_3 = xnn_fmadd_f32(vx2_3, valpha_9, valpha_7);
    vp_0 = xnn_fmadd_f32(vx2_0, vp_0, valpha_5);
    vp_1 = xnn_fmadd_f32(vx2_1, vp_1, valpha_5);
    vp_2 = xnn_fmadd_f32(vx2_2, vp_2, valpha_5);
    vp_3 = xnn_fmadd_f32(vx2_3, vp_3, valpha_5);
    vp_0 = xnn_fmadd_f32(vx2_0, vp_0, valpha_3);
    vp_1 = xnn_fmadd_f32(vx2_1, vp_1, valpha_3);
    vp_2 = xnn_fmadd_f32(vx2_2, vp_2, valpha_3);
    vp_3 = xnn_fmadd_f32(vx2_3, vp_3, valpha_3);
    vp_0 = xnn_fmadd_f32(vx2_0, vp_0, valpha_1);
    vp_1 = xnn_fmadd_f32(vx2_1, vp_1, valpha_1);
    vp_2 = xnn_fmadd_f32(vx2_2, vp_2, valpha_1);
    vp_3 = xnn_fmadd_f32(vx2_3, vp_3, valpha_1);
    vp_0 = xnn_mul_f32(vx_0, vp_0);
    vp_1 = xnn_mul_f32(vx_1, vp_1);
    vp_2 = xnn_mul_f32(vx_2, vp_2);
    vp_3 = xnn_mul_f32(vx_3, vp_3);

    // Evaluate the denominator polynomial q.
    xnn_simd_f32_t vq_0 = xnn_fmadd_f32(vx2_0, vbeta_8, vbeta_6);
    xnn_simd_f32_t vq_1 = xnn_fmadd_f32(vx2_1, vbeta_8, vbeta_6);
    xnn_simd_f32_t vq_2 = xnn_fmadd_f32(vx2_2, vbeta_8, vbeta_6);
    xnn_simd_f32_t vq_3 = xnn_fmadd_f32(vx2_3, vbeta_8, vbeta_6);
    vq_0 = xnn_fmadd_f32(vx2_0, vq_0, vbeta_4);
    vq_1 = xnn_fmadd_f32(vx2_1, vq_1, vbeta_4);
    vq_2 = xnn_fmadd_f32(vx2_2, vq_2, vbeta_4);
    vq_3 = xnn_fmadd_f32(vx2_3, vq_3, vbeta_4);
    vq_0 = xnn_fmadd_f32(vx2_0, vq_0, vbeta_2);
    vq_1 = xnn_fmadd_f32(vx2_1, vq_1, vbeta_2);
    vq_2 = xnn_fmadd_f32(vx2_2, vq_2, vbeta_2);
    vq_3 = xnn_fmadd_f32(vx2_3, vq_3, vbeta_2);
    vq_0 = xnn_fmadd_f32(vx2_0, vq_0, vone);
    vq_1 = xnn_fmadd_f32(vx2_1, vq_1, vone);
    vq_2 = xnn_fmadd_f32(vx2_2, vq_2, vone);
    vq_3 = xnn_fmadd_f32(vx2_3, vq_3, vone);

    // Divide the numerator by the denominator.
    xnn_simd_f32_t vrq_0 = xnn_rcp_f32(vq_0);
    xnn_simd_f32_t vrq_1 = xnn_rcp_f32(vq_1);
    xnn_simd_f32_t vrq_2 = xnn_rcp_f32(vq_2);
    xnn_simd_f32_t vrq_3 = xnn_rcp_f32(vq_3);
    for (size_t iter = 0; iter < XNN_SIMD_NUM_RCP_ITER_F32; iter++) {
      vrq_0 = xnn_mul_f32(vrq_0, xnn_fnmadd_f32(vrq_0, vq_0, vtwo));
      vrq_1 = xnn_mul_f32(vrq_1, xnn_fnmadd_f32(vrq_1, vq_1, vtwo));
      vrq_2 = xnn_mul_f32(vrq_2, xnn_fnmadd_f32(vrq_2, vq_2, vtwo));
      vrq_3 = xnn_mul_f32(vrq_3, xnn_fnmadd_f32(vrq_3, vq_3, vtwo));
    }
    // Note that we _could_ use a fused multiply-add to compute `p * rq + 1`,
    // but we actually want this to round to zero near the edges, so we
    // don't want the extended precision of the fused multiply-add.
    const xnn_simd_f32_t verf_0 = xnn_mul_f32(vp_0, vrq_0);
    const xnn_simd_f32_t verf_1 = xnn_mul_f32(vp_1, vrq_1);
    const xnn_simd_f32_t verf_2 = xnn_mul_f32(vp_2, vrq_2);
    const xnn_simd_f32_t verf_3 = xnn_mul_f32(vp_3, vrq_3);

    // Add one to the rational interpolant, and multiply by 0.5 times the
    // original input.
    const xnn_simd_f32_t vy_0 = xnn_mul_f32(xnn_mul_f32(vx_orig_0, vhalf),
                                        xnn_add_f32(verf_0, vone));
    const xnn_simd_f32_t vy_1 = xnn_mul_f32(xnn_mul_f32(vx_orig_1, vhalf),
                                        xnn_add_f32(verf_1, vone));
    const xnn_simd_f32_t vy_2 = xnn_mul_f32(xnn_mul_f32(vx_orig_2, vhalf),
                                        xnn_add_f32(verf_2, vone));
    const xnn_simd_f32_t vy_3 = xnn_mul_f32(xnn_mul_f32(vx_orig_3, vhalf),
                                        xnn_add_f32(verf_3, vone));

    xnn_storeu_f32(output, vy_0);
    xnn_storeu_f32(output + 1 * xnn_simd_size_f32, vy_1);
    xnn_storeu_f32(output + 2 * xnn_simd_size_f32, vy_2);
    xnn_storeu_f32(output + 3 * xnn_simd_size_f32, vy_3);
    output += 64;
  }
  for (; batch >= xnn_simd_bytes_f32; batch -= xnn_simd_bytes_f32) {
    const xnn_simd_f32_t vx_orig = xnn_loadu_f32(input);
    input += xnn_simd_size_f32;

    // Clamp the inputs to the interpolation range.
    xnn_simd_f32_t vx = xnn_min_f32(vmax_abs_x, vx_orig);
    vx = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx);

    // Since the polynomials are odd/even, we need x^2.
    const xnn_simd_f32_t vx2 = xnn_mul_f32(vx, vx);

    // Evaluate the numerator polynomial p.
    xnn_simd_f32_t vp = xnn_fmadd_f32(vx2, valpha_9, valpha_7);
    vp = xnn_fmadd_f32(vx2, vp, valpha_5);
    vp = xnn_fmadd_f32(vx2, vp, valpha_3);
    vp = xnn_fmadd_f32(vx2, vp, valpha_1);
    vp = xnn_mul_f32(vx, vp);

    // Evaluate the denominator polynomial q.
    xnn_simd_f32_t vq = xnn_fmadd_f32(vx2, vbeta_8, vbeta_6);
    vq = xnn_fmadd_f32(vx2, vq, vbeta_4);
    vq = xnn_fmadd_f32(vx2, vq, vbeta_2);
    vq = xnn_fmadd_f32(vx2, vq, vone);

    // Divide the numerator by the denominator and add one
    xnn_simd_f32_t vrq = xnn_rcp_f32(vq);
    for (size_t iter = 0; iter < XNN_SIMD_NUM_RCP_ITER_F32; iter++) {
      vrq = xnn_mul_f32(vrq, xnn_fnmadd_f32(vrq, vq, vtwo));
    }
    // Note that we _could_ use a fused multiply-add to compute `p * rq + 1`,
    // but we actually want this to round to zero near the edges, so we
    // don't want the extended precision of the fused multiply-add.
    const xnn_simd_f32_t verf = xnn_mul_f32(vp, vrq);

    // Add one to the rational interpolant, and multiply by 0.5 times the
    // original input.
    const xnn_simd_f32_t vy = xnn_mul_f32(xnn_mul_f32(vx_orig, vhalf),
                                          xnn_add_f32(verf, vone));

    xnn_storeu_f32(output, vy);
    output += xnn_simd_size_f32;
  }
  if XNN_UNLIKELY(batch != 0) {
    xnn_simd_f32_t vx_orig = xnn_load_tail_f32(input, batch >> XNN_LOG2_SIZEOF_FLOAT);

  // See above for comments.
  xnn_simd_f32_t vx = xnn_min_f32(vmax_abs_x, vx_orig);
  vx = xnn_max_f32(xnn_neg_f32(vmax_abs_x), vx);
  const xnn_simd_f32_t vx2 = xnn_mul_f32(vx, vx);
  xnn_simd_f32_t vp = xnn_fmadd_f32(vx2, valpha_9, valpha_7);
  vp = xnn_fmadd_f32(vx2, vp, valpha_5);
  vp = xnn_fmadd_f32(vx2, vp, valpha_3);
  vp = xnn_fmadd_f32(vx2, vp, valpha_1);
  vp = xnn_mul_f32(vx, vp);
  xnn_simd_f32_t vq = xnn_fmadd_f32(vx2, vbeta_8, vbeta_6);
  vq = xnn_fmadd_f32(vx2, vq, vbeta_4);
  vq = xnn_fmadd_f32(vx2, vq, vbeta_2);
  vq = xnn_fmadd_f32(vx2, vq, vone);
  xnn_simd_f32_t vrq = xnn_rcp_f32(vq);
  for (size_t iter = 0; iter < XNN_SIMD_NUM_RCP_ITER_F32; iter++) {
    vrq = xnn_mul_f32(vrq, xnn_fnmadd_f32(vrq, vq, vtwo));
  }
  const xnn_simd_f32_t verf = xnn_mul_f32(vp, vrq);
  const xnn_simd_f32_t vy = xnn_mul_f32(xnn_mul_f32(vx_orig, vhalf),
                                        xnn_add_f32(verf, vone));

    xnn_store_tail_f32(output, vy, batch >> XNN_LOG2_SIZEOF_FLOAT);
  }
}