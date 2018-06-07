/*
 * Merlin++: C++ Class Library for Charged Particle Accelerator Simulations
 * Copyright (c) 2001-2018 The Merlin++ developers
 * This file is covered by the terms the GNU GPL version 2, or (at your option) any later version, see the file COPYING
 * This file is derived from software bearing the copyright notice in merlin4_copyright.txt
 */

#include "OneToOneCorrection.h"
#include "ResponseMatrixGenerator.h"
#include "Accelerator.h"
#include "TLASimp.h"
#include "ILCDFS_IO.h"

using namespace TLAS;

void OneToOneCorrection(Accelerator* acc, Accelerator::Plane pxy, size_t nseg)
{
	dfs_trace(dfs_trace::level_1) << "Applying one-to-one steering to " << acc->GetName() << ": ";
	switch(pxy)
	{
	case Accelerator::x_only:
		dfs_trace(dfs_trace::level_1) << "X plane only";
		break;
	case Accelerator::y_only:
		dfs_trace(dfs_trace::level_1) << "Y plane only";
		break;
	case Accelerator::x_and_y:
		dfs_trace(dfs_trace::level_1) << "X and Y planes";
		break;
	}
	dfs_trace(dfs_trace::level_1) << endl;

	ReferenceParticleArray refp;
	acc->InitialiseTracking(1, refp);
	acc->AllowIncrementalTracking(true);

	IntegerArray ibpm;
	acc->GetBeamlineIndexes("BPM.*", ibpm);
	DFS_Segment s = acc->GetBeamlineRange();

	for(size_t n = 0; n < ibpm.size(); n += nseg)
	{

		DFS_Segment sn;
		sn.first = n == 0 ? s.first : ibpm[n] + 1;
		sn.second = n + nseg < ibpm.size() ? ibpm[n + nseg] : s.second;
		dfs_trace(dfs_trace::level_2) << "   correcting " << sn;

		ROChannelArray bpms;
		RWChannelArray xycors;
		acc->SetActiveBeamlineSegment(sn);
		acc->GetMonitorChannels(pxy, bpms);
		acc->GetCorrectorChannels(pxy, xycors);

		ResponseMatrixGenerator rmg(acc, bpms, xycors);
		SVDMatrix<double> svd(rmg.Generate(0));
		RealVector y0 = rmg.GetReference();
		RealVector y = svd(y0);
		xycors.IncrementAll(-1.0 * y);

		if(dfs_trace::verbosity >= dfs_trace::level_2)
		{
			dfs_trace(dfs_trace::level_2) << ' ' << sqrt(y0 * y0);
			acc->TrackBeam(0);
			y0 = bpms;
			dfs_trace(dfs_trace::level_2) << ' ' << sqrt(y0 * y0);
		}
		dfs_trace(dfs_trace::level_2) << endl;

		if(sn.second == s.second)
		{
			break;
		}
	}
}
