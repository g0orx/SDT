/* FMDemod.c
 *
 * based on code from Warren Pratt's WDSP
 *
 * John Melton G0ORX
 */

#ifndef BEENHERE
#include "SDT.h"
#endif

static float32_t FMD_rate = 48000.0;
static float32_t FMD_deviation = 2500.0; // NFM
static float32_t FMD_fmin = -4000.0; // NFM
static float32_t FMD_fmax = +4000.0; // NFM

static float32_t FMD_zeta = 1.0;
static float32_t FMD_Phase = 0.0;
static float32_t FMD_omega = 0.0;
static float32_t FMD_omegaN = 20000.0;
static float32_t FMD_fil_out = 0.0;
static float32_t FMD_fmdc = 0.0;
static float32_t FMD_tau = 0.02;

static float32_t FMD_omega_min;
static float32_t FMD_omega_max;
static float32_t FMD_g1;
static float32_t FMD_g2;
static float32_t FMD_mtau;
static float32_t FMD_onem_mtau;
static float32_t FMD_again;

static bool FMD_initialized = false;

void FMDemod_init() {
  FMD_omega_min = TWO_PI * FMD_fmin / FMD_rate;
  FMD_omega_max = TWO_PI * FMD_fmax / FMD_rate;
  FMD_g1 = 1.0 - expf(-2.0 * FMD_omegaN * FMD_zeta / FMD_rate);
  FMD_g2 = -FMD_g1 + 2.0 * (1 - expf(-FMD_omegaN * FMD_zeta / FMD_rate) * arm_cos_f32(FMD_omegaN / FMD_rate * sqrtf(1.0 - FMD_zeta * FMD_zeta)));
  FMD_mtau = expf(-1.0 / (FMD_rate * FMD_tau));
  FMD_onem_mtau = 1.0 - FMD_mtau;
  FMD_again = FMD_rate / (FMD_deviation * TWO_PI);
  FMD_initialized = true;
}

void FMDemod() {
  int i;
	float32_t det, del_out;
	float32_t vco[2], corr[2];

// TEMP -- need to call FMDemod_init from setup
  if(!FMD_initialized) {
    FMDemod_init();
  }
	for (i = 0; i < (int)FFT_length / 2; i++)
	{
		// pll
		vco[0]  = arm_cos_f32 (FMD_Phase);
		vco[1]  = arm_sin_f32 (FMD_Phase);
		corr[0] = + iFFT_buffer[FFT_length + i * 2] * vco[0] + iFFT_buffer[FFT_length + i * 2 + 1] * vco[1];
		corr[1] = - iFFT_buffer[FFT_length + i * 2] * vco[1] + iFFT_buffer[FFT_length + i * 2 + 1] * vco[0];
		if ((corr[0] == 0.0) && (corr[1] == 0.0)) corr[0] = 1.0;
		det = atan2 (corr[1], corr[0]);
		del_out = FMD_fil_out;
		FMD_omega += FMD_g2 * det;
		if (FMD_omega < FMD_omega_min) FMD_omega = FMD_omega_min;
		if (FMD_omega > FMD_omega_max) FMD_omega = FMD_omega_max;
		FMD_fil_out = FMD_g1 * det + FMD_omega;
		FMD_Phase += del_out;
		while (FMD_Phase >= TWO_PI) FMD_Phase -= TWO_PI;
		while (FMD_Phase < 0.0) FMD_Phase += TWO_PI;
		// dc removal, gain, & demod output
		FMD_fmdc = FMD_mtau * FMD_fmdc + FMD_onem_mtau * FMD_fil_out;
		float_buffer_R[i] = float_buffer_L[i] = FMD_again * (FMD_fil_out - FMD_fmdc);
	}

  // TODO de-emphasis, audio filter, CTCSS removal

}