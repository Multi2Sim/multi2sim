/*
 * Multi2Sim
 * Copyright (C) 2014 Agamemnon Despopoulos (agdespopoulos@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef DRAM_ACTION_H
#define DRAM_ACTION_H

#include <memory>
#include <vector>

#include <lib/cpp/IniFile.h>
#include <lib/esim/Event.h>


namespace dram
{

// Forward declarations
class Request;


class Actions
{
	// Unique instance of this class
	static std::unique_ptr<Actions> instance;

	// Private constructor, used internally to instantiate a singleton. Use
	// a call to getInstance() instead.
	Actions();

	// Private methods to help with configuration parsing.
	void ParseAction(const std::string &line);
	void ParseActionRequest(const std::vector<std::string> &tokens);
	void ParseActionDecode(const std::vector<std::string> &tokens);

public:

	/// Obtain the instance of the Actions singleton.
	static Actions *getInstance();

	void ParseConfiguration(misc::IniFile &config);

	/// Event handler that adds a request to the dram system.
	static void ActionRequestHandler(esim::EventType *,
			esim::EventFrame *);
};


class ActionRequestFrame : public esim::EventFrame
{

public:
	ActionRequestFrame(std::shared_ptr<Request> request)
			:
			request(request)
	{
	}

	std::shared_ptr<Request> request;
};

}  // namespace dram

#endif
