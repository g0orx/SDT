/* FMDemod.c
 *
 * based on code from Warren Pratt's WDSP
 *
 * John Melton G0ORX 02/21/2024
 */

#ifndef BEENHERE
#include "SDT.h"
#endif

static float32_t FM_rate = 48000.0;

static float32_t FM_deviation = 2500.0; // NFM

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

static float32_t FMM_f_low = 300.0;
static float32_t FMM_f_high = 3000.0;
static int FMM_ctcss_run = 1;
static float32_t FMM_ctcss_level = 0.1;
static float32_t FMM_ctcss_freq = 100.0;
        // for ctcss gen
static float32_t FMM_tscale;
static float32_t FMM_tphase;
static float32_t FMM_tdelta;
        // mod
static float32_t FMM_sphase;
static float32_t FMM_sdelta;
        // bandpass
static int FMM_bp_run;
static float32_t FMM_bp_fc;
static int FMM_nc;
static int FMM_mp;
/*
static FIRCORE FMM_p;
*/

static bool FMM_initialized = false;
static bool FMD_initialized = false;

void FMMod_init() {
  // ctcss gen
  FMM_tscale = 1.0 / (1.0 + FMM_ctcss_level);
  FMM_tphase = 0.0;
  FMM_tdelta = TWO_PI * FMM_ctcss_freq / FM_rate;
  // mod
  FMM_sphase = 0.0;
  FMM_sdelta = TWO_PI * FM_deviation / FM_rate;
  // bandpass
  FMM_bp_fc = FM_deviation + FMM_f_high;
  FMM_initialized = true;
}

void FMMod ()
{
  int i;
  float32_t dp, magdp, peak;

// TEMP -- need to call FMDemod_init from setup
  if(!FMM_initialized) {
    FMMod_init();
  }
  peak = 0.0;
  for (i = 0; i < 256; i++)
  {
    if (FMM_ctcss_run)
    {
      FMM_tphase += FMM_tdelta;
      if (FMM_tphase >= TWO_PI) FMM_tphase -= TWO_PI;
      float_buffer_L_EX[2 * i + 0] = FMM_tscale * (float_buffer_L_EX[2 * i + 0] + FMM_ctcss_level * arm_cos_f32 (FMM_tphase));
    }
    dp = float_buffer_L_EX[2 * i + 0] * FMM_sdelta;
    FMM_sphase += dp;
    if (FMM_sphase >= TWO_PI) FMM_sphase -= TWO_PI;
    if (FMM_sphase <   0.0 ) FMM_sphase += TWO_PI;
    float_buffer_L_EX[2 * i + 0] = 0.7071 * arm_cos_f32 (FMM_sphase);
    float_buffer_R_EX[2 * i + 0] = 0.7071 * arm_sin_f32 (FMM_sphase);
    if ((magdp = dp) < 0.0) magdp = - magdp;
    if (magdp > peak) peak = magdp;
  }
  /*
  if (FMM_>bp_run)
    xfircore (FMM_>p);
  */  
}


void FMDemod_init() {
  FMD_omega_min = TWO_PI * FMD_fmin / FM_rate;
  FMD_omega_max = TWO_PI * FMD_fmax / FM_rate;
  FMD_g1 = 1.0 - expf(-2.0 * FMD_omegaN * FMD_zeta / FM_rate);
  FMD_g2 = -FMD_g1 + 2.0 * (1 - expf(-FMD_omegaN * FMD_zeta / FM_rate) * arm_cos_f32(FMD_omegaN / FM_rate * sqrtf(1.0 - FMD_zeta * FMD_zeta)));
  FMD_mtau = expf(-1.0 / (FM_rate * FMD_tau));
  FMD_onem_mtau = 1.0 - FMD_mtau;
  FMD_again = FM_rate / (FM_deviation * TWO_PI);
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


void Squelch() {
  if(squelch!=0) {
    if(dbm < (-141+squelch)) {
      arm_scale_f32(float_buffer_L, 0.0, float_buffer_L, (int)FFT_length / 2);
      arm_scale_f32(float_buffer_R, 0.0, float_buffer_R, (int)FFT_length / 2);
    }
  }
}