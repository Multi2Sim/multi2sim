/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include "PassPool.h"


namespace comm
{


// The singleton instance of a PassPool.
std::unique_ptr<PassPool> PassPool::instance;


PassPool *PassPool::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create Instance
	instance.reset(new PassPool());
	return instance.get();
}


void PassPool::registerPass(std::unique_ptr<Pass> &&pass)
{
	// Add the given pass rvalue reference to the pass pool.
	passes[pass->getId()] = std::move(pass);
}


Pass *PassPool::getPass(int pass_id)
{
	// Return a pointer to the pass with the given pass_id.
	return passes[pass_id].get();
}


void PassPool::runAll()
{

}


}  // namespace comm
