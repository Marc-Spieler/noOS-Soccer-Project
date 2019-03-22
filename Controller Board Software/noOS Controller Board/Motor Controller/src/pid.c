/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 22.03.2019                                                  */
/************************************************************************/

#include "pid.h"

float pidReg(pidReg_t* reg, float refer, float act)
{
  float err;
  float prop;
  float diff;
  float outPreSat;
  float out;

  // Compute the error
  err = refer - act;

  // Compute the proportional output
  prop = reg->kp * err;

  // Compute the integral output
  reg->intg = reg->intg + reg->ki * err + reg->kc * reg->satErr;

  // Compute the derivative output
  diff = reg->kd * (err - reg->prevErr);

  // Compute the pre-saturated output
  outPreSat = prop + reg->intg + diff;

  // Saturate the output
  if (outPreSat > reg->outMax)
  {
    out = reg->outMax;
  }
  else if (outPreSat < reg->outMin)
  {
    out = reg->outMin;
  }
  else
  {
    out = outPreSat;
  }

  // Compute the saturate difference
  reg->satErr = out - outPreSat;

  // Keep error for next iteration
  reg->prevErr = err;
  
  return out;
}
