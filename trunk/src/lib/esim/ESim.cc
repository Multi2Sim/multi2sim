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

#include <csignal>

#include "ESim.h"


namespace esim
{

std::unique_ptr<ESim> ESim::instance;

void ESim::SignalHandler(int signum)
{
	// Get instance
	ESim *esim = getInstance();

	// If a signal SIGINT has been caught already and not processed, it is
	// time to not defer it anymore. Execution ends here.
	if (esim->signal_received == signum && signum == SIGINT)
	{
		std::cerr << "SIGINT received\n";
		exit(1);
	}

	// Just record that we are receiving a signal. It is not a good idea to
	// process it now, since we might be interfering some critical
	// execution. The signal will be processed at the end of the simulation
	// loop iteration.
	esim->signal_received = signum;
}


ESim::ESim()
	: timer("ESimTimer")
{
	// Initialize
	signal_received = 0;
	finish = ESimFinishNone;
	timer.Start();

	// Enable signal handler
	EnableSignals();
}


ESim *ESim::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();
	
	// Create instance
	instance.reset(new ESim());
	return instance.get();
}


void ESim::EnableSignals()
{
	signal(SIGINT, &SignalHandler);
	signal(SIGABRT, &SignalHandler);
	signal(SIGUSR1, &SignalHandler);
	signal(SIGUSR2, &SignalHandler);
}


void ESim::DisableSignals()
{
	signal(SIGABRT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGUSR1, SIG_DFL);
	signal(SIGUSR2, SIG_DFL);
}


void ESim::ProcessEvents()
{
	// Check for Control+C interrupt
	if (signal_received == SIGINT)
	{
		std::cerr << "\nSignal SIGINT received\n";
		finish = ESimFinishSignal;
	}
}


}  // namespace esim

