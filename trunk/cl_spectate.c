#include "quakedef.h"


cvar_t	cl_spectate_smoothing_allow = { "cl_spectate_smoothing_allow", "1" };
cvar_t	cl_spectate_smoothing = { "cl_spectate_smoothing", "0.5" };

// Variables to try to automatically figure out whether we are spectating. This
// counts how many times svc_setangle was called within the
// min_rate_considered_spectating. That way the user does not have to toggle
// a variable do disable the smoothing when playing again to unlock the
// viewangle.
// If the detection generates a false positive there is still
// cl_spectate_smoothing_allow to disable this completely.
static const double min_rate_considered_spectating = 0.1;
static const long min_num_considered_spectating = 3;
static long last_num_within_rate = 0;
static double last_setangle_time = -1.0;


static qboolean CL_ResetIfDefinitelyNotSpectating (void)
{
	if (last_setangle_time > cl.time ||
	    ((cl.time - last_setangle_time) > min_rate_considered_spectating))
	{
		last_num_within_rate = 0;
		last_setangle_time = -1.0;
		return true;
	}

	return false;
}

qboolean CL_SpectateIsSmoothingEnabled (void)
{
	if (!cl_spectate_smoothing_allow.value)
		return false;
	if (CL_ResetIfDefinitelyNotSpectating())
		return false;
	return last_num_within_rate >= 3;
}

void CL_SpectateViewangleSmoothing (void)
{
	extern cvar_t cl_spectate;
	extern cvar_t cl_spectate_smoothing;
	const float max_angle_smoothed = 45.0f;
	const float max_smoothing = 0.75f;
	const float smoothing_factor = bound(0.0f, cl_spectate_smoothing.value, max_smoothing);

	if (!cl_spectate_smoothing_allow.value)
		return;

	last_setangle_time = cl.time;
	if (CL_ResetIfDefinitelyNotSpectating())
		return;
	last_num_within_rate += 1;

	for (int i = 0 ; i < 3 ; i++)
	{
		float d = cl.viewangles[i] - cl.mviewangles[0][i];
		if (d > 180)
			d -= 360;
		else if (d < -180)
			d += 360;
		if (abs(d) <= max_angle_smoothed)
			cl.viewangles[i] -= smoothing_factor * d;
	}
	VectorCopy(cl.mviewangles[0], cl.mviewangles[1]);
	VectorCopy(cl.viewangles, cl.mviewangles[0]);
}

void CL_InitSpectate (void)
{
	Cvar_Register(&cl_spectate_smoothing_allow);
	Cvar_Register(&cl_spectate_smoothing);
}
