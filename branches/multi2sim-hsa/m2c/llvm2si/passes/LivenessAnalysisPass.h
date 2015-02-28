/*
 *  Multi2Sim
 *  Copyright (C) 2014  Charu Kalra (ckalra@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef M2C_LLVM2SI_PASSES_LIVENESS_ANALYSIS_PASS_H
#define M2C_LLVM2SI_PASSES_LIVENESS_ANALYSIS_PASS_H

#include <memory>
#include <unordered_map>

#include <m2c/common/Pass.h>
#include <m2c/common/PassInfo.h>
#include <m2c/llvm2si/Function.h>
#include <src/lib/cpp/Bitmap.h>
#include <src/lib/cpp/Misc.h>


namespace llvm2si
{

class BasicBlockLivenessAnalysisPassInfo : public comm::BasicBlockPassInfo
{
	misc::Bitmap *vector_def_info;
	misc::Bitmap *vector_use_info;
	misc::Bitmap *vector_in_info;
	misc::Bitmap *vector_out_info;

	misc::Bitmap *scalar_def_info;
	misc::Bitmap *scalar_use_info;
	misc::Bitmap *scalar_in_info;
	misc::Bitmap *scalar_out_info;

public:

	void ResetVectorDefInfo(int size)
	{
		vector_def_info = new misc::Bitmap(size);
		vector_def_info->Reset();
	}

	void ResetVectorUseInfo(int size)
	{
		vector_use_info =  new misc::Bitmap(size);
		vector_use_info->Reset();
	}

	void ResetVectorInInfo(int size)
	{
		vector_in_info = new misc::Bitmap(size);
		vector_in_info->Reset();
	}

	void ResetVectorOutInfo(int size)
	{
		vector_out_info =  new misc::Bitmap(size);
		vector_out_info->Reset();
	}

	void SetVectorDefInfo(misc::Bitmap *dummy_vdef)
	{
		*vector_def_info = *dummy_vdef;
	}

	void SetVectorUseInfo(misc::Bitmap *dummy_vuse)
	{
		*vector_use_info = *dummy_vuse;
	}

	void SetVectorInInfo(misc::Bitmap *dummy_vin)
	{
		*vector_in_info = *dummy_vin;
	}

	void SetVectorOutInfo(misc::Bitmap *dummy_vout)
	{
		*vector_out_info = *dummy_vout;
	}

	misc::Bitmap GetVectorDefInfo()
	{
		return *vector_def_info;
	}

	misc::Bitmap GetVectorUseInfo()
	{
		return *vector_use_info;
	}

	misc::Bitmap GetVectorInInfo()
	{
		return *vector_in_info;
	}

	misc::Bitmap GetVectorOutInfo()
	{
		return *vector_out_info;
	}

	void ResetScalarDefInfo(int size)
	{
		scalar_def_info = new misc::Bitmap(size);
		scalar_def_info->Reset();
	}

	void ResetScalarUseInfo(int size)
	{
		scalar_use_info =  new misc::Bitmap(size);
		scalar_use_info->Reset();
	}

	void ResetScalarInInfo(int size)
	{
		scalar_in_info = new misc::Bitmap(size);
		scalar_in_info->Reset();
	}

	void ResetScalarOutInfo(int size)
	{
		scalar_out_info = new misc::Bitmap(size);
		scalar_out_info->Reset();
	}

	void SetScalarDefInfo(misc::Bitmap *dummy_sdef)
	{
		*scalar_def_info = *dummy_sdef;
	}

	void SetScalarUseInfo(misc::Bitmap *dummy_suse)
	{
		*scalar_use_info = *dummy_suse;
	}

	void SetScalarInInfo(misc::Bitmap *dummy_sin)
	{
		*scalar_in_info = *dummy_sin;
	}

	void SetScalarOutInfo(misc::Bitmap *dummy_sout)
	{
		*scalar_out_info = *dummy_sout;
	}

	misc::Bitmap GetScalarDefInfo()
	{
		return *scalar_def_info;
	}

	misc::Bitmap GetScalarUseInfo()
	{
		return *scalar_use_info;
	}

	misc::Bitmap GetScalarInInfo()
	{
		return *scalar_in_info;
	}

	misc::Bitmap GetScalarOutInfo()
	{
		return *scalar_out_info;
	}

	~BasicBlockLivenessAnalysisPassInfo()
	{
		delete vector_def_info;
		delete vector_use_info;
		delete vector_in_info;
		delete vector_out_info;

		delete scalar_def_info;
		delete scalar_use_info;
		delete scalar_in_info;
		delete scalar_out_info;
	}
};

class LivenessAnalysisPass : public comm::Pass
{
	/// Function to run LivenessAnalysis on
	llvm2si::Function *lap_function;

	public:

		/// Constructor
		LivenessAnalysisPass(llvm2si::Function *function) :
			lap_function(function)
		{

		}

		/// Return a pointer of BasicBlockLivenessAnalysisPassInfo
		template<typename ConcreteType> ConcreteType* getInfo(BasicBlock *basic_block)
		{
			return basic_block->getPassInfoPool()->get<ConcreteType>(getId());
		}

		/// Execute the liveness analysis pass
		void run();

		/// Dump debug information related to liveness analysis pass
		void dump();

		~LivenessAnalysisPass() {}
};

}

#endif
